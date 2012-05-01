#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <event.h>
#include <mem.h>

#include "minunit.h"

//==============================================================================
//
// Globals
//
//==============================================================================

struct tagbstring foo = bsStatic("foo");
struct tagbstring bar = bsStatic("bar");
struct tagbstring baz = bsStatic("baz");

int ACTION_EVENT_DATA_LENGTH = 13;
char ACTION_EVENT_DATA[] = {
    0x01, 0x00, 0xD0, 0x90, 0x96, 0x34, 0x01, 0x00,
    0x00, 0x14, 0x00, 0x00, 0x00
};

int DATA_EVENT_DATA_LENGTH = 23;
char DATA_EVENT_DATA[] = {
    0x02, 0x00, 0xD0, 0x90, 0x96, 0x34, 0x01, 0x00,
    0x00, 0x0C, 0x00, 0x01, 0x00, 0x03, 0x66, 0x6F,
    0x6F, 0x02, 0x00, 0x03, 0x62, 0x61, 0x72
};

int ACTION_DATA_EVENT_DATA_LENGTH = 27;
char ACTION_DATA_EVENT_DATA[] = {
    0x03, 0x00, 0xD0, 0x90, 0x96, 0x34, 0x01, 0x00,
    0x00, 0x14, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x01,
    0x00, 0x03, 0x66, 0x6F, 0x6F, 0x02, 0x00, 0x03,
    0x62, 0x61, 0x72
};


//==============================================================================
//
// Test Cases
//
//==============================================================================

//--------------------------------------
// Lifecycle
//--------------------------------------

int test_sky_event_create() {
    sky_event *event = sky_event_create(1325376000000LL, 10, 200);
    mu_assert(event != NULL, "Unable to allocate event");
    mu_assert(event->timestamp == 1325376000000LL, "Event timestamp not assigned");
    mu_assert(event->object_id == 10, "Event object id not assigned");
    mu_assert(event->action_id == 200, "Event action id not assigned");
    mu_assert(event->data == NULL, "Event data non-null");
    mu_assert(event->data_count == 0, "Event data count not initialized");

    sky_event_free(event);

    return 0;
}


//--------------------------------------
// Event data
//--------------------------------------

int test_sky_event_set_data() {
    sky_event_data *data = NULL;

    sky_event *event = sky_event_create(0, 0, 0);
    sky_event_set_data(event, 10, &foo);
    sky_event_set_data(event, 20, &bar);
    sky_event_set_data(event, 10, &baz);
    mu_assert(event->data_count == 2, "Expected data count to be 2");

    sky_event_get_data(event, 10, &data);
    mu_assert(biseqcstr(data->value, "baz"), "Expected data 10 to equal 'baz'");
    sky_event_get_data(event, 20, &data);
    mu_assert(biseqcstr(data->value, "bar"), "Expected data 20 to equal 'bar'");
    sky_event_get_data(event, 30, &data);
    mu_assert(data == NULL, "Expected data 30 be NULL");

    sky_event_free(event);

    return 0;
}

int test_sky_event_unset_data() {
    sky_event_data *data = NULL;

    sky_event *event = sky_event_create(0, 0, 0);
    sky_event_set_data(event, 10, &foo);
    sky_event_set_data(event, 20, &bar);
    mu_assert(event->data_count == 2, "Expected data count to be 2");
    sky_event_unset_data(event, 10);
    mu_assert(event->data_count == 1, "Expected data count to be 1");

    sky_event_get_data(event, 10, &data);
    mu_assert(data == NULL, "Expected data 10 be NULL");
    sky_event_get_data(event, 20, &data);
    mu_assert(biseqcstr(data->value, "bar"), "Expected data 20 to equal 'bar'");

    sky_event_free(event);

    return 0;
}


//--------------------------------------
// Serialization length
//--------------------------------------

// Action-only event.
int test_sky_event_action_event_get_serialized_length() {
    sky_event *event = sky_event_create(1325376000000LL, 0, 20);
    mu_assert(sky_event_get_serialized_length(event) == 13, "Unexpected length for action-only event.");
    sky_event_free(event);
    return 0;
}

// Data-only event.
int test_sky_event_data_event_get_serialized_length() {
    sky_event *event = sky_event_create(1325376000000LL, 0, 0);
    sky_event_set_data(event, 1, &foo);
    sky_event_set_data(event, 2, &bar);
    mu_assert(sky_event_get_serialized_length(event) == 23, "Unexpected length for data-only event.");
    sky_event_free(event);
    return 0;
}

// Action + data event.
int test_sky_event_action_data_event_get_serialized_length() {
    sky_event *event = sky_event_create(1325376000000LL, 0, 100);
    sky_event_set_data(event, 1, &foo);
    sky_event_set_data(event, 2, &bar);
    mu_assert(sky_event_get_serialized_length(event) == 27, "Unexpected length for action+data event.");
    sky_event_free(event);
    return 0;
}

//--------------------------------------
// Serialization
//--------------------------------------

// Action event.
int test_sky_event_action_event_serialize() {
    ptrdiff_t ptrdiff;
    void *addr = calloc(ACTION_EVENT_DATA_LENGTH, 1);
    sky_event *event = sky_event_create(1325376000000LL, 0, 20);
    sky_event_serialize(event, addr, &ptrdiff);
    sky_event_free(event);
    mu_assert(ptrdiff == ACTION_EVENT_DATA_LENGTH, "");
    mu_assert(memcmp(addr, &ACTION_EVENT_DATA, ACTION_EVENT_DATA_LENGTH) == 0, "");
    free(addr);
    return 0;
}

// Data event.
int test_sky_event_data_event_serialize() {
    ptrdiff_t ptrdiff;
    void *addr = calloc(DATA_EVENT_DATA_LENGTH, 1);
    sky_event *event = sky_event_create(1325376000000LL, 0, 0);
    sky_event_set_data(event, 1, &foo);
    sky_event_set_data(event, 2, &bar);
    sky_event_serialize(event, addr, &ptrdiff);
    sky_event_free(event);
    mu_assert(ptrdiff == DATA_EVENT_DATA_LENGTH, "");
    mu_assert(memcmp(addr, &DATA_EVENT_DATA, DATA_EVENT_DATA_LENGTH) == 0, "");
    free(addr);
    return 0;
}

// Action+Data event.
int test_sky_event_action_data_event_serialize() {
    ptrdiff_t ptrdiff;
    void *addr = calloc(ACTION_DATA_EVENT_DATA_LENGTH, 1);
    sky_event *event = sky_event_create(1325376000000LL, 0, 20);
    sky_event_set_data(event, 1, &foo);
    sky_event_set_data(event, 2, &bar);
    sky_event_serialize(event, addr, &ptrdiff);
    sky_event_free(event);
    mu_assert(ptrdiff == ACTION_DATA_EVENT_DATA_LENGTH, "");
    mu_assert(memcmp(addr, &ACTION_DATA_EVENT_DATA, ACTION_DATA_EVENT_DATA_LENGTH) == 0, "");
    free(addr);
    return 0;
}

//--------------------------------------
// Deserialization
//--------------------------------------

// Action event.
int test_sky_event_action_event_deserialize() {
    ptrdiff_t ptrdiff;
    sky_event *event = sky_event_create(0, 0, 0);
    sky_event_deserialize(event, &ACTION_EVENT_DATA, &ptrdiff);

    mu_assert(ptrdiff == ACTION_EVENT_DATA_LENGTH, "");
    mu_assert(event->timestamp == 1325376000000LL, "Expected timestamp to equal 1325376000000LL");
    mu_assert(event->action_id == 20, "Expected action id to equal 20");
    mu_assert(event->object_id == 0, "Expected object id to equal 0");
    mu_assert(event->data == NULL, "Expected data to be NULL");
    mu_assert(event->data_count == 0, "Expected data count to be 0");

    sky_event_free(event);
    
    return 0;
}

// Data event.
int test_sky_event_data_event_deserialize() {
    ptrdiff_t ptrdiff;
    sky_event *event = sky_event_create(0, 0, 0);
    sky_event_deserialize(event, &DATA_EVENT_DATA, &ptrdiff);

    mu_assert(ptrdiff == DATA_EVENT_DATA_LENGTH, "");
    mu_assert(event->timestamp == 1325376000000LL, "Expected timestamp to equal 1325376000000LL");
    mu_assert(event->action_id == 0, "Expected action id to equal 0");
    mu_assert(event->object_id == 0, "Expected object id to equal 0");
    mu_assert(event->data != NULL, "Expected data to not be NULL");
    mu_assert(event->data_count == 2, "Expected data count to be 2");

    sky_event_data *data = NULL;
    sky_event_get_data(event, 1, &data);
    mu_assert(biseqcstr(data->value, "foo"), "Expected data 1 to equal 'foo'");
    sky_event_get_data(event, 2, &data);
    mu_assert(biseqcstr(data->value, "bar"), "Expected data 1 to equal 'bar'");

    sky_event_free(event);
    
    return 0;
}

// Action+Data event.
int test_sky_event_action_data_event_deserialize() {
    ptrdiff_t ptrdiff;
    sky_event *event = sky_event_create(0, 0, 0);
    sky_event_deserialize(event, &ACTION_DATA_EVENT_DATA, &ptrdiff);

    mu_assert(ptrdiff == ACTION_DATA_EVENT_DATA_LENGTH, "");
    mu_assert(event->timestamp == 1325376000000LL, "Expected timestamp to equal 1325376000000LL");
    mu_assert(event->action_id == 20, "Expected action id to equal 20");
    mu_assert(event->object_id == 0, "Expected object id to equal 0");
    mu_assert(event->data != NULL, "Expected data to not be NULL");
    mu_assert(event->data_count == 2, "Expected data count to be 2");

    sky_event_data *data = NULL;
    sky_event_get_data(event, 1, &data);
    mu_assert(biseqcstr(data->value, "foo"), "Expected data 1 to equal 'foo'");
    sky_event_get_data(event, 2, &data);
    mu_assert(biseqcstr(data->value, "bar"), "Expected data 1 to equal 'bar'");

    sky_event_free(event);
    
    return 0;
}



//==============================================================================
//
// Setup
//
//==============================================================================

int all_tests() {
    mu_run_test(test_sky_event_create);

    mu_run_test(test_sky_event_set_data);
    mu_run_test(test_sky_event_unset_data);
    
    mu_run_test(test_sky_event_action_event_get_serialized_length);
    mu_run_test(test_sky_event_data_event_get_serialized_length);
    mu_run_test(test_sky_event_action_data_event_get_serialized_length);
    
    mu_run_test(test_sky_event_action_event_serialize);
    mu_run_test(test_sky_event_data_event_serialize);
    mu_run_test(test_sky_event_action_data_event_serialize);

    mu_run_test(test_sky_event_action_event_deserialize);
    mu_run_test(test_sky_event_data_event_deserialize);
    mu_run_test(test_sky_event_action_data_event_deserialize);

    return 0;
}

RUN_TESTS()