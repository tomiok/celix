/**
 *Licensed to the Apache Software Foundation (ASF) under one
 *or more contributor license agreements.  See the NOTICE file
 *distributed with this work for additional information
 *regarding copyright ownership.  The ASF licenses this file
 *to you under the Apache License, Version 2.0 (the
 *"License"); you may not use this file except in compliance
 *with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *Unless required by applicable law or agreed to in writing,
 *software distributed under the License is distributed on an
 *"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 *specific language governing permissions and limitations
 *under the License.
 */
/*
 * manifest.h
 *
 *  Created on: Jul 5, 2010
 *      Author: alexanderb
 */

#ifndef MANIFEST_H_
#define MANIFEST_H_

#include <apr_general.h>

#include "properties.h"
#include "celix_errno.h"

struct manifest {
	apr_pool_t *pool;
	PROPERTIES mainAttributes;
	HASH_MAP attributes;
};

typedef struct manifest * MANIFEST;

celix_status_t manifest_create(apr_pool_t *pool, MANIFEST *manifest);
celix_status_t manifest_createFromFile(apr_pool_t *pool, char *filename, MANIFEST *manifest);

void manifest_clear(MANIFEST manifest);
PROPERTIES manifest_getMainAttributes(MANIFEST manifest);
celix_status_t manifest_getEntries(MANIFEST manifest, HASH_MAP *map);

celix_status_t manifest_read(MANIFEST manifest, char *filename);
void manifest_write(MANIFEST manifest, char * filename);

char * manifest_getValue(MANIFEST manifest, const char * name);

#endif /* MANIFEST_H_ */