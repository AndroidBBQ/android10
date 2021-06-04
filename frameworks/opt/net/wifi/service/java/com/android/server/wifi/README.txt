Path history for this code:

commit date: 2013-12-18 to 2014-01-07
commit hash: a07c419913bfae2a896fbc29e8f269ee08c4d910 (add)
commit hash: 4a3f9cf099bbbe52dc0edb2a7e1d1c976bc335a3 (delete)
dst:         frameworks/opt/net/wifi/service
src:         frameworks/base/services/core/java/com/android/server/wifi

commit date: 2013-12-19
commit hash: 9158825f9c41869689d6b1786d7c7aa8bdd524ce (many more files)
commit hash: 19c662b3df3b35756a92282bb6cc767e6407cb8a (a few files)
dst:         frameworks/base/services/core/java/com/android/server/wifi
src:         frameworks/base/services/java/com/android/server/wifi

commit date: 2013-12-11
commit hash: ffadfb9ffdced62db215319d3edc7717802088fb
dst:         frameworks/base/services/java/com/android/server/wifi
src:         frameworks/base/wifi/java/android/net/wifi

commit date: 2008-10-21
commit hash: 54b6cfa9a9e5b861a9930af873580d6dc20f773c
dst:         frameworks/base/wifi/java/android/net/wifi
src:         initial aosp import?

////////////////////////////////////////////////////////////////

Salient points about Wifi Service implementation

WifiService: Implements the IWifiManager 3rd party API. The API and the device
state information (screen on/off, battery state, sleep policy) go as input into
the WifiController which tracks high level states as to whether STA or AP mode
is operational and controls wifi to handle bringup and shut down.

WifiController: Acts as a controller to the ActiveModeWarden based on various inputs (API and device state). Runs on the same thread created in WifiService.

WifiSettingsStore: Tracks the various settings (wifi toggle, airplane toggle, tethering toggle, scan mode toggle) and provides API to figure if wifi should be turned on or off.

WifiTrafficPoller: Polls traffic on wifi and notifies apps listening on it.

WifiNotificationController: Controls whether the open network notification is displayed or not based on the scan results.

ActiveModeWarden: Tracks the various states on STA and AP connectivity and handles bring up and shut down.

Feature description:

Scan-only mode with Wi-Fi turned off:
 - Setup wizard opts user into allowing scanning for improved location. We show no further dialogs in setup wizard since the user has just opted into the feature. This is the reason WifiService listens to DEVICE_PROVISIONED setting.
 - Once the user has his device provisioned, turning off Wi-Fi from settings or from a third party app will show up a dialog reminding the user that scan mode will be on even though Wi-Fi is being turned off. The user has the choice to turn this notification off.
 - In the scan mode, the device continues to allow scanning from any app with Wi-Fi turned off. This is done by disabling all networks and allowing only scans to be passed.
