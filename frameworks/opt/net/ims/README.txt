This package provides access to IP Multimedia Subsystem (IMS) functionality,
especially making and taking VoLTE calls using IMS.
At the moment, this only supports VoLTE calls which is compliant to GSMA IR.92 specification.

For the additional information, you can refer the below standard specifications.
    - GSMA IR.92 : features for voice and sms profile
    - GSMA IR.94 : video calling feature
    - 3GPP TS 24.229 : IMS call control (SIP and SDP)
    - 3GPP TS 26.114 : IMS media handling and interaction
    - 3GPP TS 26.111 : Codec for CS multimedia telephony service (H.324)
    - 3GPP TS 24.623 : XCAP over the Ut interface for manipulating supplementary services
      (XCAP : XML Configuration Access Protocol)

To get started, you need to get an instance of the ImsManager by calling ImsManager#getInstance().

With the ImsManager, you can initiate VoLTE calls with ImsManager#makeCall()
and ImsManager#takeCall(). Both methods require a ImsCall#Listener that
receives callbacks when the state of the call changes, such as
when the call is ringing, established, or ended.

ImsManager#makeCall() requires an ImsCallProfile objects, representing the call properties
of the local device. ImsCallProfile can creates by ImsManager
using the specified service and call type.
ImsCallProfile is created by referring GSMA IR.92, GSMA IR.94, 3GPP TS 24.229,
3GPP TS 26.114 and 3GPP TS 26.111.

To receive calls, an IMS application MUST provide a BroadcastReceiver that
has the ability to respond to an intent indicating that there is an incoming call.
The default action for the incoming call intent is ImsManager#ACTION_IMS_INCOMING_CALL.
And, the application frees to define the action for the incoming call intent.

There are two packages for IMS APIs.

    - com.android.ims
        It provides the functionalities for the upper layer of IMS APIs.
        In this moment, it is used by the VoLTE Phone application.

    - com.android.ims.internal
        It provides the functionalities for the internal usage or lower layer of IMS APIs.
        It needs to be implemented by the IMS protocol solution vendor.



== Classes for com.android.ims ==

ImsManager
    Provides APIs for IMS services, such as initiating IMS calls, and provides access to
    the mobile operator's IMS network. This class is the starting point for any IMS actions.

ImsCall
    Provides IMS voice / video calls over LTE network.

ImsCallGroup
    Manages all IMS calls which are established hereafter the initial 1-to-1 call is established.
    It's for providing the dummy calls which are disconnected with the IMS network after
    merged or extended to the conference.

ImsCallProfile
    Parcelable object to handle IMS call profile
    It provides the service and call type, the additional information related to the call.

ImsConferenceState
    It provides the conference information (defined in RFC 4575) for IMS conference call.

ImsConnectionStateListener
    It is a listener type for receiving notifications about changes to the IMS connection.
    It provides a state of IMS registration between UE and network, the service availability of
    the local device during IMS registered.

ImsException
    It provides a general IMS-related exception.

ImsReasonInfo
    It enables an application to get details on why a method call failed.

ImsServiceClass
    It defines an identifier for each IMS service category.

ImsStreamMediaProfile
    Parcelable object to handle IMS stream media profile.
    It provides the media direction, quality of audio and/or video.

ImsUtInterface
    It provides APIs for the supplementary service settings using IMS (Ut interface, XCAP).



== Classes for com.android.ims.internal ==

CallGroup
    Wrapper class which has an ICallGroup interface.

CallGroupBase
    Implements ICallGroup interface.
    Manages all calls which are established hereafter the initial 1-to-1 call is established.
    It's for providing the dummy calls which are disconnected with the IMS network after
    merged or extended to the conference.

CallGroupManager
    Manages CallGroup objects.

ICallGroup
    Provides the interface to manage all calls which are established hereafter the initial
    1-to-1 call is established. It's for providing the dummy calls which are disconnected with
    the IMS network after merged or extended to the conference.

ImsCallSession
    Provides the call initiation/termination, and media exchange between two IMS endpoints.
    It directly communicates with IMS service which implements the IMS protocol behavior.

ImsStreamMediaSession
    Provides the APIs to control the media session, such as passing the surface object,
    controlling the camera (front/rear selection, zoom, brightness, ...) for a video calling.