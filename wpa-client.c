/** Code originally taken from @morandg:
 * https://stackoverflow.com/questions/8521661/d-bus-tutorial-in-c-to-communicate-with-wpa-supplicant
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include <dbus/dbus.h>

#include "wpa-client.h"

int running = 1;

void stopLoop(int sig) {
    running = 0;
}

void sendScan(){
  // TODO !
}


/** Main even loop */
void loop(DBusConnection* conn) {
    DBusMessage* msg;
    DBusMessageIter args;
    int argType, buffSize = 1024;
    const char* member = 0;

    sendScan();

    while (running) {
        // non blocking read of the next available message
        dbus_connection_read_write(conn, 0);
        msg = dbus_connection_pop_message(conn);

        // loop again if we haven't read a message
        if (!msg) {
            printf("No message received, waiting a little ...\n");
            sleep(1);
            continue;
        } else
            printf("Got a message, will analyze it ...\n");

        // Print the message member
        printf("Got message for interface %s\n",
                dbus_message_get_interface(msg));

        member = dbus_message_get_member(msg);
        if (member)
            printf("Got message member %s\n", member);

        // Check has argument
        if (!dbus_message_iter_init(msg, &args)) {
            printf("Message has no argument\n");
            continue;
        } else {
            // Go through arguments
            while (1) {
                argType = dbus_message_iter_get_arg_type(&args);

                if (argType == DBUS_TYPE_STRING) {
                    printf("Got string argument, extracting ...\n");
                    char* strBuffer = NULL;
                    dbus_message_iter_get_basic(&args, &strBuffer);
                    printf("Received string: \n %s \n",strBuffer);

                } else if (argType == DBUS_TYPE_ARRAY) {
                    printf("Signature of message: %s\n", dbus_message_iter_get_signature(&args));
                    DBusMessageIter iter_dict, iter_dict_entry, iter_dict_content;
                    char* key = NULL;
                    char* value = NULL;
                    int i = 0;

                    dbus_message_iter_recurse(&args, &iter_dict);

                    while (dbus_message_iter_get_arg_type(&iter_dict) == DBUS_TYPE_DICT_ENTRY) {
                        printf("Iteration: %d\n", i);
                        printf("Outer container is %d\n", dbus_message_iter_get_arg_type(&iter_dict));
                        dbus_message_iter_recurse(&iter_dict, &iter_dict_entry);
                        printf("Inner container is: %d\n", dbus_message_iter_get_arg_type(&iter_dict_entry));
                        dbus_message_iter_next(&iter_dict);
                        dbus_message_iter_get_basic(&iter_dict_entry, &key);
                        printf("Received key: \n %s \n", key);

                        if (dbus_message_iter_get_arg_type(&iter_dict) == DBUS_TYPE_VARIANT) {
                            //dbus_message_iter_open_container(&iter_dict_entry, DBUS_TYPE_VARIANT, NULL, &iter_dict_content);
                            dbus_message_iter_get_basic(&iter_dict_content, &value);
                            printf("Received value: \n %s \n", value);
                        }
                        i++;
                    }
                    printf("Exited because arg type was: %d\n", dbus_message_iter_get_arg_type(&iter_dict));
                    i = 0;
                    printf("Exit loop, type is %d\n", dbus_message_iter_get_arg_type(&iter_dict));
                } else
                    printf("Arg type %d not implemented yet !\n", argType);

                if (dbus_message_iter_has_next(&args))
                    dbus_message_iter_next(&args);
                else
                    break;
            }

            printf("No more arguments!\n");
        }

        // free the message
        dbus_message_unref(msg);
    }
}

int main(int argc, char* argv[]) {
    DBusError err;
    DBusConnection* conn;
    int ret;
    char signalDesc[1024];     // Signal description as string

    // Signal handling
    signal(SIGKILL, stopLoop);
    signal(SIGTERM, stopLoop);

    // Initialize err struct
    dbus_error_init(&err);

    // connect to the bus
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    if (!conn) {
        exit(1);
    }

    // request a name on the bus
    ret = dbus_bus_request_name(conn, WPAS_DBUS_SERVICE, 0, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Name Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    /**
     * Add all signal matching rules
     */

    // Rule: Match interface signal
    sprintf(signalDesc, "type='signal',interface='%s'",
            WPAS_DBUS_IFACE_INTERFACE);
    dbus_bus_add_match(conn, signalDesc, &err);
    dbus_connection_flush(conn);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Match Error (%s)\n", err.message);
        exit(1);
    }

    // Rule: Match network signal
    sprintf(signalDesc, "type='signal',interface='%s'",
            WPAS_DBUS_IFACE_NETWORK);
    dbus_bus_add_match(conn, signalDesc, &err);
    dbus_connection_flush(conn);
    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Match Error (%s)\n", err.message);
        exit(1);
    }

    // Rule: Match Bssid signal
    sprintf(signalDesc, "type='signal',interface='%s'",
            WPAS_DBUS_IFACE_BSSID);
    dbus_bus_add_match(conn, signalDesc, &err);
    dbus_connection_flush(conn);
    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Match Error (%s)\n", err.message);
        exit(1);
    }

    // Do main loop
    loop(conn);

    // Main loop exited
    printf("Main loop stopped, exiting ...\n");

    dbus_connection_close(conn);

    return 0;
}


/**
 * Start reading from a dbus dict.
 *
 * @param iter A valid DBusMessageIter pointing to the start of the dict
 * @param iter_dict (out) A DBusMessageIter to be passed to
 *    wpa_dbus_dict_read_next_entry()
 * @error on failure a descriptive error
 * @return TRUE on success, FALSE on failure
 *
 */
dbus_bool_t wpa_dbus_dict_open_read(DBusMessageIter *iter,
				    DBusMessageIter *iter_dict,
				    DBusError *error)
{
	int type;

	printf("%s: start reading a dict entry", __func__);
	if (!iter || !iter_dict) {
		dbus_set_error_const(error, DBUS_ERROR_FAILED,
				     "[internal] missing message iterators");
		return FALSE;
	}

	type = dbus_message_iter_get_arg_type(iter);
	if (type != DBUS_TYPE_ARRAY ||
	    dbus_message_iter_get_element_type(iter) != DBUS_TYPE_DICT_ENTRY) {
		printf("%s: unexpected message argument types (arg=%c element=%c)",
			   __func__, type,
			   type != DBUS_TYPE_ARRAY ? '?' :
			   dbus_message_iter_get_element_type(iter));
		dbus_set_error_const(error, DBUS_ERROR_INVALID_ARGS,
				     "unexpected message argument types");
		return FALSE;
	}

	dbus_message_iter_recurse(iter, iter_dict);
	return TRUE;
}
