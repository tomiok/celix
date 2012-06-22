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

#include <stdlib.h>
#include <apr_strings.h>

#include <bundle_activator.h>
#include <bundle_context.h>
#include <celixbool.h>
#include <service_tracker.h>

#include "driver_locator.h"
#include "device_manager.h"

struct device_manager_bundle_instance {
	BUNDLE_CONTEXT context;
	apr_pool_t *pool;
	device_manager_t deviceManager;
	SERVICE_TRACKER driverLocatorTracker;
	SERVICE_TRACKER driverTracker;
	SERVICE_TRACKER deviceTracker;
};

typedef struct device_manager_bundle_instance *device_manager_bundle_instance_t;

static celix_status_t deviceManagerBundle_createDriverLocatorTracker(device_manager_bundle_instance_t bundleData);
static celix_status_t deviceManagerBundle_createDriverTracker(device_manager_bundle_instance_t bundleData);
static celix_status_t deviceManagerBundle_createDeviceTracker(device_manager_bundle_instance_t bundleData);

celix_status_t addingService_dummy_func(void * handle, SERVICE_REFERENCE reference, void **service) {
	celix_status_t status = CELIX_SUCCESS;
	device_manager_t dm = handle;
	BUNDLE_CONTEXT context = NULL;
	status = deviceManager_getBundleContext(dm, &context);
	if (status == CELIX_SUCCESS) {
		status = bundleContext_getService(context, reference, service);
	}
	return status;
}

celix_status_t bundleActivator_create(BUNDLE_CONTEXT context, void **userData) {
	celix_status_t status = CELIX_SUCCESS;
	apr_pool_t *pool;
	status = bundleContext_getMemoryPool(context, &pool);
	if (status == CELIX_SUCCESS) {
		device_manager_bundle_instance_t bi = apr_palloc(pool, sizeof(struct device_manager_bundle_instance));
		if (bi == NULL) {
			status = CELIX_ENOMEM;
		} else {
			(*userData) = bi;
			bi->context = context;
			bi->pool = pool;
			status = deviceManager_create(pool, context, &bi->deviceManager);
		}
	}
	return status;
}

celix_status_t bundleActivator_start(void * userData, BUNDLE_CONTEXT context) {
	celix_status_t status = CELIX_SUCCESS;
	device_manager_bundle_instance_t bundleData = userData;
	apr_pool_t *pool;
	status = bundleContext_getMemoryPool(context, &pool);
	if (status == CELIX_SUCCESS) {
		status = deviceManagerBundle_createDriverLocatorTracker(bundleData);
		if (status == CELIX_SUCCESS) {
			status = deviceManagerBundle_createDriverTracker(bundleData);
			if (status == CELIX_SUCCESS) {
					status = deviceManagerBundle_createDeviceTracker(bundleData);
					if (status == CELIX_SUCCESS) {
						status = serviceTracker_open(bundleData->driverLocatorTracker);
						if (status == CELIX_SUCCESS) {
							status = serviceTracker_open(bundleData->driverTracker);
							if (status == CELIX_SUCCESS) {
								status = serviceTracker_open(bundleData->deviceTracker);
							}
						}
					}
			}
		}
	}

	if (status != CELIX_SUCCESS) {
		printf("DEVICE_MANAGER: Error while starting bundle got error num %i\n", status);
	}

	printf("DEVICE_MANAGER: Started\n");
	return status;
}

static celix_status_t deviceManagerBundle_createDriverLocatorTracker(device_manager_bundle_instance_t bundleData) {
	celix_status_t status = CELIX_SUCCESS;
	SERVICE_TRACKER_CUSTOMIZER customizer = apr_palloc(bundleData->pool, sizeof(struct serviceTrackerCustomizer));
	if (customizer != NULL) {
		customizer->handle=bundleData->deviceManager;
		customizer->addingService=addingService_dummy_func;
		customizer->addedService=deviceManager_locatorAdded;
		customizer->modifiedService=deviceManager_locatorModified;
		customizer->removedService=deviceManager_locatorRemoved;

		SERVICE_TRACKER tracker = NULL;
		status = serviceTracker_create(bundleData->pool, bundleData->context, "driver_locator", customizer, &tracker);
		if (status == CELIX_SUCCESS) {
			bundleData->driverLocatorTracker=tracker;
		}
	} else {
		status = CELIX_ENOMEM;
	}
	return status;
}

static celix_status_t deviceManagerBundle_createDriverTracker(device_manager_bundle_instance_t bundleData) {
	celix_status_t status = CELIX_SUCCESS;
	SERVICE_TRACKER_CUSTOMIZER customizer = apr_palloc(bundleData->pool, sizeof(struct serviceTrackerCustomizer));
	if (customizer != NULL) {
		customizer->handle=bundleData->deviceManager;
		customizer->addingService=addingService_dummy_func;
		customizer->addedService=deviceManager_driverAdded;
		customizer->modifiedService=deviceManager_driverModified;
		customizer->removedService=deviceManager_driverRemoved;

		SERVICE_TRACKER tracker = NULL;
		status = serviceTracker_createWithFilter(bundleData->pool, bundleData->context, "(objectClass=driver)", customizer, &tracker);
		if (status == CELIX_SUCCESS) {
			bundleData->driverTracker=tracker;
		}
	} else {
		status = CELIX_ENOMEM;
	}
	return status;
}

static celix_status_t deviceManagerBundle_createDeviceTracker(device_manager_bundle_instance_t bundleData) {
	celix_status_t status = CELIX_SUCCESS;
	SERVICE_TRACKER_CUSTOMIZER customizer = apr_palloc(bundleData->pool, sizeof(struct serviceTrackerCustomizer));
	if (customizer != NULL) {
		customizer->handle=bundleData->deviceManager;
		customizer->addingService=addingService_dummy_func;
		customizer->addedService= deviceManager_deviceAdded;
		customizer->modifiedService=deviceManager_deviceModified;
		customizer->removedService=deviceManager_deviceRemoved;

		SERVICE_TRACKER tracker = NULL;
		status = serviceTracker_createWithFilter(bundleData->pool, bundleData->context, "(|(objectClass=device)(DEVICE_CATEGORY=*))", customizer, &tracker);
		if (status == CELIX_SUCCESS) {
			bundleData->deviceTracker=tracker;
		}
	} else {
		status = CELIX_ENOMEM;
	}
	return status;
}

celix_status_t bundleActivator_stop(void * userData, BUNDLE_CONTEXT context) {
	celix_status_t status = CELIX_SUCCESS;
	device_manager_bundle_instance_t bundleData = userData;
//	status = serviceTracker_close(bundleData->driverLocatorTracker);
	if (status == CELIX_SUCCESS) {
		status = serviceTracker_close(bundleData->driverTracker);
		if (status == CELIX_SUCCESS) {
			status = serviceTracker_close(bundleData->deviceTracker);
		}
	}
	return status;
}

celix_status_t bundleActivator_destroy(void * userData, BUNDLE_CONTEXT context) {
	celix_status_t status = CELIX_SUCCESS;
	device_manager_bundle_instance_t bundleData = userData;
	deviceManager_destroy(bundleData->deviceManager);
	return CELIX_SUCCESS;
}