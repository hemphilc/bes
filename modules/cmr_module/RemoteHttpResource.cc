// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of cmr_MODULE, A C++ MODULE that can be loaded in to
// the OPeNDAP Back-End Server (BES) and is able to handle remote requests.

// Copyright (c) 2013 OPeNDAP, Inc.
// Author: Nathan Potter <ndp@opendap.org>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.

// Authors:
//      ndp       Nathan Potter <ndp@opendap.org>

#include "config.h"

#include <sstream>

#include "BESInternalError.h"

#include "BESDebug.h"
#include "BESUtil.h"

#include "CmrCache.h"
#include "CmrUtils.h"
#include "curl_utils.h"
#include "RemoteHttpResource.h"

using namespace std;
using namespace cmr;

#define MODULE  "cmr"

/**
 * Builds a RemoteHttpResource object associated with the passed \c url parameter.
 *
 * @param url Is a URL string that identifies the remote resource.
 */
RemoteHttpResource::RemoteHttpResource(const string &url)
{
    d_initialized = false;

    d_fd = 0;
    d_curl = 0;
    d_resourceCacheFileName.clear();
    d_response_headers = new vector<string>();
    d_request_headers = new vector<string>();

    if (url.empty()) {
        string err = "RemoteHttpResource(): Remote resource URL is empty";
        throw BESInternalError(err, __FILE__, __LINE__);
    }

    d_remoteResourceUrl = url;

    BESDEBUG(MODULE, "RemoteHttpResource() - URL: " << d_remoteResourceUrl << endl);

    // EXAMPLE: returned value parameter for CURL *
    //
    // CURL *www_lib_init(CURL **curl); // function type signature
    //
    // CURL *pvparam = 0;               // passed value parameter
    // result = www_lib_init(&pvparam); // the call to the method

    d_curl = init(d_error_buffer);  // This may throw either Error or InternalErr

    configureProxy(d_curl, d_remoteResourceUrl); // Configure the a proxy for this url (if appropriate).

    BESDEBUG(MODULE, "RemoteHttpResource() - d_curl: " << d_curl << endl);
}

/**
 * Releases any memory resources and also any existing cache file locks for the cached resource.
 * ( Closes the file descriptor opened when retrieveResource() was called.)
 */RemoteHttpResource::~RemoteHttpResource()
{
    BESDEBUG(MODULE, "~RemoteHttpResource() - BEGIN   resourceURL: " << d_remoteResourceUrl << endl);

    delete d_response_headers;
    d_response_headers = 0;
    BESDEBUG(MODULE, "~RemoteHttpResource() - Deleted d_response_headers." << endl);

    delete d_request_headers;
    d_request_headers = 0;
    BESDEBUG(MODULE, "~RemoteHttpResource() - Deleted d_request_headers." << endl);

    if (!d_resourceCacheFileName.empty()) {
        CmrCache *cache = CmrCache::get_instance();
        if (cache) {
            cache->unlock_and_close(d_resourceCacheFileName);
            BESDEBUG(MODULE, "~RemoteHttpResource() - Closed and unlocked "<< d_resourceCacheFileName << endl);
            d_resourceCacheFileName.clear();
        }
    }

    if (d_curl) {
        curl_easy_cleanup(d_curl);
        BESDEBUG(MODULE, "~RemoteHttpResource() - Called curl_easy_cleanup()." << endl);
    }
    d_curl = 0;

    BESDEBUG(MODULE, "~RemoteHttpResource() - END   resourceURL: " << d_remoteResourceUrl << endl);
    d_remoteResourceUrl.clear();
}

/**
 * This method will check the cache for the resource. If it's not there then it will lock the cache and retrieve
 * the remote resource content using HTTP GET.
 *
 * When this method returns the RemoteHttpResource object is fully initialized and the cache file name for the resource
 * is available along with an open file descriptor for the (now read-locked) cache file.
 */
void RemoteHttpResource::retrieveResource()
{
    BESDEBUG(MODULE,
        "RemoteHttpResource::retrieveResource() - BEGIN   resourceURL: " << d_remoteResourceUrl << endl);

    if (d_initialized) {
        BESDEBUG(MODULE, "RemoteHttpResource::retrieveResource() - END  Already initialized." << endl);
        return;
    }

    // Get a pointer to the singleton cache instance for this process.
    CmrCache *cache = CmrCache::get_instance();
    if (!cache) {
        ostringstream oss;
        oss << __func__ << "() - FAILED to get local cache."
            " Unable to proceed with request for " << this->d_remoteResourceUrl
            << " The gateway_MODULE MUST have a valid cache configuration to operate." << endl;
        BESDEBUG(MODULE, oss.str());
        throw BESInternalError(oss.str(), __FILE__, __LINE__);
    }

    // Get the name of the file in the cache (either the code finds this file or
    // or it makes it).
    d_resourceCacheFileName = cache->get_cache_file_name(d_remoteResourceUrl);
    BESDEBUG(MODULE,
        "RemoteHttpResource::retrieveResource() - d_resourceCacheFileName: " << d_resourceCacheFileName << endl);

    // @TODO MAKE THIS RETRIEVE THE CACHED DATA TYPE IF THE CACHED RESPONSE IF FOUND
    // We need to know the type of the resource. HTTP headers are the preferred  way to determine the type.
    // Unfortunately, the current code losses both the HTTP headers sent from the request and the derived type
    // to subsequent accesses of the cached object. Since we have to have a type, for now we just set the type
    // from the url. If down below we DO an HTTP GET then the headers will be evaluated and the type set by setType()
    // But really - we gotta fix this.
    CmrUtils::Get_type_from_url(d_remoteResourceUrl, d_type);
    BESDEBUG(MODULE, "RemoteHttpResource::retrieveResource() - d_type: " << d_type << endl);

    try {

        if (cache->get_read_lock(d_resourceCacheFileName, d_fd)) {
            BESDEBUG(MODULE,
                "RemoteHttpResource::retrieveResource() - Remote resource is already in cache. cache_file_name: " << d_resourceCacheFileName << endl);
            d_initialized = true;
            return;
        }

        // Now we actually need to reach out across the interwebs and retrieve the remote resource and put it's
        // content into a local cache file, given that it's not in the cache.
        // First make an empty file and get an exclusive lock on it.
        if (cache->create_and_lock(d_resourceCacheFileName, d_fd)) {

            // Write the remote resource to the cache file.
            writeResourceToFile(d_fd);

            // #########################################################################################################
            // I think right here is where I would be able to cache the data type/response headers. While I have
            // the exclusive lock I could open another cache file for metadata and write to it.
            // #########################################################################################################

            // Change the exclusive lock on the new file to a shared lock. This keeps
            // other processes from purging the new file and ensures that the reading
            // process can use it.
            cache->exclusive_to_shared_lock(d_fd);
            BESDEBUG(MODULE,
                "RemoteHttpResource::retrieveResource() - Converted exclusive cache lock to shared lock." << endl);

            // Now update the total cache size info and purge if needed. The new file's
            // name is passed into the purge method because this process cannot detect its
            // own lock on the file.
            unsigned long long size = cache->update_cache_info(d_resourceCacheFileName);
            BESDEBUG(MODULE, "RemoteHttpResource::retrieveResource() - Updated cache info" << endl);

            if (cache->cache_too_big(size)) {
                cache->update_and_purge(d_resourceCacheFileName);
                BESDEBUG(MODULE, "RemoteHttpResource::retrieveResource() - Updated and purged cache." << endl);
            }

            BESDEBUG(MODULE, "RemoteHttpResource::retrieveResource() - END" << endl);

            d_initialized = true;

            return;
        }
        else {
            if (cache->get_read_lock(d_resourceCacheFileName, d_fd)) {
                BESDEBUG(MODULE,
                    "RemoteHttpResource::retrieveResource() - Remote resource is in cache. cache_file_name: " << d_resourceCacheFileName << endl);
                d_initialized = true;
                return;
            }
        }

        string msg = "RemoteHttpResource::retrieveResource() - Failed to acquire cache read lock for remote resource: '";
        msg += d_remoteResourceUrl + "\n";
        throw libdap::Error(msg);

    }
    catch (...) {
        BESDEBUG(MODULE,
            "RemoteHttpResource::retrieveResource() - Caught exception, unlocking cache and re-throw." << endl);
        cache->unlock_cache();
        throw;
    }

}

/**
 *
 * Retrieves the remote resource and write it the the open file associated with the open file
 * descriptor parameter 'fd'. In the process of caching the file a FILE * is fdopen'd from 'fd' and that is used buy
 * curl to write the content. At the end the stream is rewound and the FILE * pointer is returned.
 *
 * @param fd An open file descriptor the is associated with the target file.
 */
void RemoteHttpResource::writeResourceToFile(int fd)
{
    BESDEBUG(MODULE, "RemoteHttpResource::writeResourceToFile() - BEGIN" << endl);

    int status = -1;
    try {
        BESDEBUG(MODULE,
            "RemoteHttpResource::writeResourceToFile() - Saving resource " << d_remoteResourceUrl << " to cache file " << d_resourceCacheFileName << endl);
        status = read_url(d_curl, d_remoteResourceUrl, fd, d_response_headers, d_request_headers,
            d_error_buffer); // Throws Error.
        if (status >= 400) {
            BESDEBUG(MODULE,
                "RemoteHttpResource::writeResourceToFile() - HTTP returned an error status: " << status << endl);
            // delete resp_hdrs; resp_hdrs = 0;
            string msg = "Error while reading the URL: '";
            msg += d_remoteResourceUrl;
            msg += "'The HTTP request returned a status of " + libdap::long_to_string(status) + " which means '";
            msg += http_status_to_string(status) + "' \n";
            throw libdap::Error(msg);
        }
        BESDEBUG(MODULE,
            "RemoteHttpResource::writeResourceToFile() - Resource " << d_remoteResourceUrl << " saved to cache file " << d_resourceCacheFileName << endl);

        // rewind the file
        // FIXME I think the idea here is that we have the file open and we should just keep
        // reading from it. But the container mechanism works with file names, so we will
        // likely have to open the file again. If that's true, lets remove this call. jhrg 3.2.18
        int status = lseek(fd, 0, SEEK_SET);
        if (-1 == status)
            throw BESError("Could not seek within the response.", BES_NOT_FOUND_ERROR, __FILE__, __LINE__);

        BESDEBUG(MODULE, "RemoteHttpResource::writeResourceToFile() - Reset file descriptor." << endl);

        // @TODO CACHE THE DATA TYPE OR THE HTTP HEADERS SO WHEN WE ARE RETRIEVING THE CACHED OBJECT WE CAN GET THE CORRECT TYPE
        setType(d_response_headers);
    }
    catch (libdap::Error &e) {
        throw;
    }
    BESDEBUG(MODULE, "RemoteHttpResource::writeResourceToFile() - END" << endl);
}

void RemoteHttpResource::setType(const vector<string> *resp_hdrs)
{

    BESDEBUG(MODULE, "RemoteHttpResource::setType() - BEGIN" << endl);

    string type = "";

    // Try and figure out the file type first from the
    // Content-Disposition in the http header response.
    string disp;
    string ctype;

    if (resp_hdrs) {
        vector<string>::const_iterator i = resp_hdrs->begin();
        vector<string>::const_iterator e = resp_hdrs->end();
        for (; i != e; i++) {
            string hdr_line = (*i);

            BESDEBUG(MODULE, "RemoteHttpResource::setType() - Evaluating header: " << hdr_line << endl);

            hdr_line = BESUtil::lowercase(hdr_line);

            string colon_space = ": ";
            int index = hdr_line.find(colon_space);
            string hdr_name = hdr_line.substr(0, index);
            string hdr_value = hdr_line.substr(index + colon_space.length());

            BESDEBUG(MODULE,
                "RemoteHttpResource::setType() - hdr_name: '" << hdr_name << "'   hdr_value: '" <<hdr_value << "' "<< endl);

            if (hdr_name.find("content-disposition") != string::npos) {
                // Content disposition exists
                BESDEBUG(MODULE, "RemoteHttpResource::setType() - Located content-disposition header." << endl);
                disp = hdr_value;
            }
            if (hdr_name.find("content-type") != string::npos) {
                BESDEBUG(MODULE, "RemoteHttpResource::setType() - Located content-type header." << endl);
                ctype = hdr_value;
            }
        }
    }

    if (!disp.empty()) {
        // Content disposition exists, grab the filename
        // attribute
        CmrUtils::Get_type_from_disposition(disp, type);
        BESDEBUG(MODULE,
            "RemoteHttpResource::setType() - Evaluated content-disposition '" << disp << "' matched type: \"" << type << "\"" << endl);
    }

    // still haven't figured out the type. Check the content-type
    // next, translate to the BES MODULE name. It's also possible
    // that even though Content-disposition was available, we could
    // not determine the type of the file.
    if (type.empty() && !ctype.empty()) {
        CmrUtils::Get_type_from_content_type(ctype, type);
        BESDEBUG(MODULE,
            "RemoteHttpResource::setType() - Evaluated content-type '" << ctype << "' matched type \"" << type << "\"" << endl);
    }

    // still haven't figured out the type. Now check the actual URL
    // and see if we can't match the URL to a MODULE name
    if (type.empty()) {
        CmrUtils::Get_type_from_url(d_remoteResourceUrl, type);
        BESDEBUG(MODULE,
            "RemoteHttpResource::setType() - Evaluated url '" << d_remoteResourceUrl << "' matched type: \"" << type << "\"" << endl);
    }

    // still couldn't figure it out, punt
    if (type.empty()) {
        string err = (string) "RemoteHttpResource::setType() - Unable to determine the type of data"
            + " returned from '" + d_remoteResourceUrl + "'  Setting type to 'unknown'";
        BESDEBUG(MODULE, err);

        type = "unknown";
        //throw BESSyntaxUserError( err, __FILE__, __LINE__ ) ;
    }

    // @TODO CACHE THE DATA TYPE OR THE HTTP HEADERS SO WHEN WE ARE RETRIEVING THE CACHED OBJECT WE CAN GET THE CORRECT TYPE

    d_type = type;

    BESDEBUG(MODULE, "RemoteHttpResource::setType() - END" << endl);

}