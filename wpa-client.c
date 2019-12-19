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

                    /* FIXME : got weird characters
                    dbus_message_iter_get_basic(&args, &strValue);
                    */

                    /* FIXME : segmentation fault !
                    dbus_message_iter_get_fixed_array(
                            &args, &strValue, buffSize);
                    */

                    /* FIXME : segmentation fault !
                    dbus_message_iter_recurse(&args, &subArgs);
                    */

                    /* FIXME : deprecated!
                    if(dbus_message_iter_get_array_len(&args) > buffSize)
                        printf("message content to big for local buffer!");
                    */

                    //printf("String value was %s\n", strValue);
                } else
                    printf("Arg type not implemented yet !\n");

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
