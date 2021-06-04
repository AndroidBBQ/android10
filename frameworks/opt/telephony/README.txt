This package contains classes used to manage a DataConnection.

A criticial aspect of this class is that most objects in this
package run on the same thread except DataConnectionTracker
This makes processing efficient as it minimizes context
switching and it eliminates issues with multi-threading.

This can be done because all actions are either asynchronous
or are known to be non-blocking and fast. At this time only
DcTesterDeactivateAll takes specific advantage of this
single threading knowledge by using Dcc#mDcListAll so be
very careful when making changes that break this assumption.

A related change was in DataConnectionAc I added code that
checks to see if the caller is on a different thread. If
it is then the AsyncChannel#sendMessageSynchronously is
used. If the caller is on the same thread then a getter
is used. This allows the DCAC to be used from any thread
and was required to fix a bug when Dcc called
PhoneBase#notifyDataConnection which calls DCT#getLinkProperties
and DCT#getLinkCapabilities which call Dcc all on the same
thread. Without this change there was a dead lock when
sendMessageSynchronously blocks.


== Testing ==

The following are Intents that can be sent for testing pruproses on
DEBUGGABLE builds (userdebug, eng)

*) Causes bringUp and retry requests to fail for all DC's

  adb shell am broadcast -a com.android.internal.telephony.dataconnection.action_fail_bringup --ei counter 2 --ei fail_cause -3

*) Causes all DC's to get torn down, simulating a temporary network outage:

  adb shell am broadcast -a com.android.internal.telephony.dataconnection.action_deactivate_all

*) To simplify testing we also have detach and attach simulations below where {x} is gsm, cdma or sip

  adb shell am broadcast -a com.android.internal.telephony.{x}.action_detached
  adb shell am broadcast -a com.android.internal.telephony.{x}.action_attached


== System properties for Testing ==

On debuggable builds (userdebug, eng) you can change additional
settings through system properties.  These properties can be set with
"setprop" for the current boot, or added to local.prop to persist
across boots.

device# setprop key value

device# echo "key=value" >> /data/local.prop
device# chmod 644 /data/local.prop


-- Retry configuration --

You can replace the connection retry configuration.  For example, you
could change it to perform 4 retries at 5 second intervals:

device# setprop test.data_retry_config "5000,5000,5000"


-- Roaming --

You can force the telephony stack to always assume that it's roaming
to verify higher-level framework functionality:

device# setprop telephony.test.forceRoaming true
