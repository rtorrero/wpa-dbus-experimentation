
#ifndef WPA_CLIENT_H
#define WPA_CLIENT_H

#define WPAS_DBUS_SERVICE   "fi.w1.wpa_supplicant1"
#define WPAS_DBUS_PATH      "/fi/w1/wpa_supplicant1"
#define WPAS_DBUS_INTERFACE "fi.w1.wpa_supplicant1"

#define WPAS_DBUS_PATH_INTERFACES   WPAS_DBUS_PATH "/Interfaces"
#define WPAS_DBUS_IFACE_INTERFACE   WPAS_DBUS_INTERFACE ".Interface"

#define WPAS_DBUS_NETWORKS_PART "Networks"
#define WPAS_DBUS_IFACE_NETWORK WPAS_DBUS_INTERFACE ".Network"

#define WPAS_DBUS_BSSIDS_PART   "BSSIDs"
#define WPAS_DBUS_IFACE_BSSID   WPAS_DBUS_INTERFACE ".BSSID"
#endif /* WPA_CLIENT_H */