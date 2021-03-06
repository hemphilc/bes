// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of the BES

// Copyright (c) 2020 OPeNDAP, Inc.
// Author: Nathan Potter<ndp@opendap.org>
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
// Created by ndp on 12/11/19.
//

#ifndef HYRAX_CREDENTIALSMANAGER_H
#define HYRAX_CREDENTIALSMANAGER_H

#include <string>
#include <vector>


// These are the names of the bes keys used to configure the handler.
#define CATALOG_MANAGER_CREDENTIALS "CredentialsManager.config"

class AccessCredentials {
public:
    // These are the string keys used to express the normative key names
    // for the credentials components.
    static const std::string ID_KEY;
    static const std::string KEY_KEY;
    static const std::string REGION_KEY;
    static const std::string BUCKET_KEY;
    static const std::string URL_KEY;
private:
    std::map<std::string, std::string> kvp;
    bool s3_tested, is_s3;
    std::string d_config_name;
public:
    AccessCredentials()= default;
    AccessCredentials(std::string config_name){ d_config_name = config_name;}
    AccessCredentials(const AccessCredentials &ac) = default;

    std::string get(const std::string &key);
    void add(const std::string &key, const std::string &value);
    bool isS3Cred();
    std::string to_json();
    std::string name(){ return d_config_name; }
};



class CredentialsManager {
public:
    static const std::string ENV_ID_KEY;
    static const std::string ENV_ACCESS_KEY;
    static const std::string ENV_REGION_KEY;
    static const std::string ENV_BUCKET_KEY;
    static const std::string ENV_URL_KEY;
    static const std::string ENV_CREDS_KEY_VALUE;

private:
    std::map<std::string, AccessCredentials* > creds;
    CredentialsManager();
    static void initialize_instance();
    static void delete_instance();

    static AccessCredentials *load_credentials_from_env( );

public:
    static CredentialsManager *theMngr;

    ~CredentialsManager();

    static CredentialsManager *theCM(){
        if (theMngr == 0) initialize_instance();
        return theMngr;
    }

    void add(const std::string &url, AccessCredentials *ac);
    static void load_credentials();
    static void clear(){ delete_instance(); }

    AccessCredentials *get(const std::string &url);


    unsigned int size(){
        return creds.size();
    }

};





#endif //HYRAX_CREDENTIALSMANAGER_H
