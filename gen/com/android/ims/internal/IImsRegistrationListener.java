/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.ims.internal;
/**
 * A listener type for receiving notifications about the changes to
 * the IMS connection(registration).
 *
 * {@hide}
 */
public interface IImsRegistrationListener extends android.os.IInterface
{
  /** Default implementation for IImsRegistrationListener. */
  public static class Default implements com.android.ims.internal.IImsRegistrationListener
  {
    /**
         * Notifies the application when the device is connected to the IMS network.
         *
         * @deprecated see {@link registrationConnectedWithRadioTech}
         */
    @Override public void registrationConnected() throws android.os.RemoteException
    {
    }
    /**
         * Notifies the application when the device is trying to connect the IMS network.
         *
         * @deprecated see {@link registrationProgressingWithRadioTech}
         */
    @Override public void registrationProgressing() throws android.os.RemoteException
    {
    }
    /**
         * Notifies the application when the device is connected to the IMS network.
         *
         * @param imsRadioTech the radio access technology. Valid values are {@code
         * RIL_RADIO_TECHNOLOGY_*} defined in {@link ServiceState}.
         */
    @Override public void registrationConnectedWithRadioTech(int imsRadioTech) throws android.os.RemoteException
    {
    }
    /**
         * Notifies the application when the device is trying to connect the IMS network.
         *
         * @param imsRadioTech the radio access technology. Valid values are {@code
         * RIL_RADIO_TECHNOLOGY_*} defined in {@link ServiceState}.
         */
    @Override public void registrationProgressingWithRadioTech(int imsRadioTech) throws android.os.RemoteException
    {
    }
    /**
         * Notifies the application when the device is disconnected from the IMS network.
         */
    @Override public void registrationDisconnected(android.telephony.ims.ImsReasonInfo imsReasonInfo) throws android.os.RemoteException
    {
    }
    /**
         * Notifies the application when its suspended IMS connection is resumed,
         * meaning the connection now allows throughput.
         */
    @Override public void registrationResumed() throws android.os.RemoteException
    {
    }
    /**
         * Notifies the application when its current IMS connection is suspended,
         * meaning there is no data throughput.
         */
    @Override public void registrationSuspended() throws android.os.RemoteException
    {
    }
    /**
         * Notifies the application when its current IMS connection is updated
         * since the service setting is changed or the service is added/removed.
         *
         * @param serviceClass a service class specified in {@link ImsServiceClass}
         * @param event an event type when this callback is called
         *    If {@code event} is 0, meaning the specified service is removed from the IMS connection.
         *    Else ({@code event} is 1), meaning the specified service is added to the IMS connection.
         */
    @Override public void registrationServiceCapabilityChanged(int serviceClass, int event) throws android.os.RemoteException
    {
    }
    /**
         * Notifies the application when features on a particular service enabled or
         * disabled successfully based on user preferences.
         *
         * @param serviceClass a service class specified in {@link ImsServiceClass}
         * @param enabledFeatures features enabled as defined in com.android.ims.ImsConfig#FeatureConstants.
         * @param disabledFeatures features disabled as defined in com.android.ims.ImsConfig#FeatureConstants.
         */
    @Override public void registrationFeatureCapabilityChanged(int serviceClass, int[] enabledFeatures, int[] disabledFeatures) throws android.os.RemoteException
    {
    }
    /**
         * Updates the application with the waiting voice message count.
         * @param count The number of waiting voice messages.
         */
    @Override public void voiceMessageCountUpdate(int count) throws android.os.RemoteException
    {
    }
    /**
         * Notifies the application when the list of URIs associated with IMS client is updated.
         */
    @Override public void registrationAssociatedUriChanged(android.net.Uri[] uris) throws android.os.RemoteException
    {
    }
    /**
         * Notifies the application when IMS registration attempt on a target
         * access tech fails.
         *
         * @param targetAccessTech Radio access technology on which the IMS registration was
         *         attempted.
         * @param imsReasonInfo Reason for the failure.
         */
    @Override public void registrationChangeFailed(int targetAccessTech, android.telephony.ims.ImsReasonInfo imsReasonInfo) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.ims.internal.IImsRegistrationListener
  {
    private static final java.lang.String DESCRIPTOR = "com.android.ims.internal.IImsRegistrationListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.ims.internal.IImsRegistrationListener interface,
     * generating a proxy if needed.
     */
    public static com.android.ims.internal.IImsRegistrationListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.ims.internal.IImsRegistrationListener))) {
        return ((com.android.ims.internal.IImsRegistrationListener)iin);
      }
      return new com.android.ims.internal.IImsRegistrationListener.Stub.Proxy(obj);
    }
    @Override public android.os.IBinder asBinder()
    {
      return this;
    }
    @Override public boolean onTransact(int code, android.os.Parcel data, android.os.Parcel reply, int flags) throws android.os.RemoteException
    {
      java.lang.String descriptor = DESCRIPTOR;
      switch (code)
      {
        case INTERFACE_TRANSACTION:
        {
          reply.writeString(descriptor);
          return true;
        }
        case TRANSACTION_registrationConnected:
        {
          data.enforceInterface(descriptor);
          this.registrationConnected();
          return true;
        }
        case TRANSACTION_registrationProgressing:
        {
          data.enforceInterface(descriptor);
          this.registrationProgressing();
          return true;
        }
        case TRANSACTION_registrationConnectedWithRadioTech:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.registrationConnectedWithRadioTech(_arg0);
          return true;
        }
        case TRANSACTION_registrationProgressingWithRadioTech:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.registrationProgressingWithRadioTech(_arg0);
          return true;
        }
        case TRANSACTION_registrationDisconnected:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsReasonInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsReasonInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.registrationDisconnected(_arg0);
          return true;
        }
        case TRANSACTION_registrationResumed:
        {
          data.enforceInterface(descriptor);
          this.registrationResumed();
          return true;
        }
        case TRANSACTION_registrationSuspended:
        {
          data.enforceInterface(descriptor);
          this.registrationSuspended();
          return true;
        }
        case TRANSACTION_registrationServiceCapabilityChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.registrationServiceCapabilityChanged(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_registrationFeatureCapabilityChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int[] _arg1;
          _arg1 = data.createIntArray();
          int[] _arg2;
          _arg2 = data.createIntArray();
          this.registrationFeatureCapabilityChanged(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_voiceMessageCountUpdate:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.voiceMessageCountUpdate(_arg0);
          return true;
        }
        case TRANSACTION_registrationAssociatedUriChanged:
        {
          data.enforceInterface(descriptor);
          android.net.Uri[] _arg0;
          _arg0 = data.createTypedArray(android.net.Uri.CREATOR);
          this.registrationAssociatedUriChanged(_arg0);
          return true;
        }
        case TRANSACTION_registrationChangeFailed:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.ims.ImsReasonInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telephony.ims.ImsReasonInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.registrationChangeFailed(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.ims.internal.IImsRegistrationListener
    {
      private android.os.IBinder mRemote;
      Proxy(android.os.IBinder remote)
      {
        mRemote = remote;
      }
      @Override public android.os.IBinder asBinder()
      {
        return mRemote;
      }
      public java.lang.String getInterfaceDescriptor()
      {
        return DESCRIPTOR;
      }
      /**
           * Notifies the application when the device is connected to the IMS network.
           *
           * @deprecated see {@link registrationConnectedWithRadioTech}
           */
      @Override public void registrationConnected() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registrationConnected, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registrationConnected();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the application when the device is trying to connect the IMS network.
           *
           * @deprecated see {@link registrationProgressingWithRadioTech}
           */
      @Override public void registrationProgressing() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registrationProgressing, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registrationProgressing();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the application when the device is connected to the IMS network.
           *
           * @param imsRadioTech the radio access technology. Valid values are {@code
           * RIL_RADIO_TECHNOLOGY_*} defined in {@link ServiceState}.
           */
      @Override public void registrationConnectedWithRadioTech(int imsRadioTech) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(imsRadioTech);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registrationConnectedWithRadioTech, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registrationConnectedWithRadioTech(imsRadioTech);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the application when the device is trying to connect the IMS network.
           *
           * @param imsRadioTech the radio access technology. Valid values are {@code
           * RIL_RADIO_TECHNOLOGY_*} defined in {@link ServiceState}.
           */
      @Override public void registrationProgressingWithRadioTech(int imsRadioTech) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(imsRadioTech);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registrationProgressingWithRadioTech, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registrationProgressingWithRadioTech(imsRadioTech);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the application when the device is disconnected from the IMS network.
           */
      @Override public void registrationDisconnected(android.telephony.ims.ImsReasonInfo imsReasonInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((imsReasonInfo!=null)) {
            _data.writeInt(1);
            imsReasonInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_registrationDisconnected, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registrationDisconnected(imsReasonInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the application when its suspended IMS connection is resumed,
           * meaning the connection now allows throughput.
           */
      @Override public void registrationResumed() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registrationResumed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registrationResumed();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the application when its current IMS connection is suspended,
           * meaning there is no data throughput.
           */
      @Override public void registrationSuspended() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registrationSuspended, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registrationSuspended();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the application when its current IMS connection is updated
           * since the service setting is changed or the service is added/removed.
           *
           * @param serviceClass a service class specified in {@link ImsServiceClass}
           * @param event an event type when this callback is called
           *    If {@code event} is 0, meaning the specified service is removed from the IMS connection.
           *    Else ({@code event} is 1), meaning the specified service is added to the IMS connection.
           */
      @Override public void registrationServiceCapabilityChanged(int serviceClass, int event) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(serviceClass);
          _data.writeInt(event);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registrationServiceCapabilityChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registrationServiceCapabilityChanged(serviceClass, event);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the application when features on a particular service enabled or
           * disabled successfully based on user preferences.
           *
           * @param serviceClass a service class specified in {@link ImsServiceClass}
           * @param enabledFeatures features enabled as defined in com.android.ims.ImsConfig#FeatureConstants.
           * @param disabledFeatures features disabled as defined in com.android.ims.ImsConfig#FeatureConstants.
           */
      @Override public void registrationFeatureCapabilityChanged(int serviceClass, int[] enabledFeatures, int[] disabledFeatures) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(serviceClass);
          _data.writeIntArray(enabledFeatures);
          _data.writeIntArray(disabledFeatures);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registrationFeatureCapabilityChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registrationFeatureCapabilityChanged(serviceClass, enabledFeatures, disabledFeatures);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Updates the application with the waiting voice message count.
           * @param count The number of waiting voice messages.
           */
      @Override public void voiceMessageCountUpdate(int count) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(count);
          boolean _status = mRemote.transact(Stub.TRANSACTION_voiceMessageCountUpdate, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().voiceMessageCountUpdate(count);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the application when the list of URIs associated with IMS client is updated.
           */
      @Override public void registrationAssociatedUriChanged(android.net.Uri[] uris) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedArray(uris, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registrationAssociatedUriChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registrationAssociatedUriChanged(uris);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the application when IMS registration attempt on a target
           * access tech fails.
           *
           * @param targetAccessTech Radio access technology on which the IMS registration was
           *         attempted.
           * @param imsReasonInfo Reason for the failure.
           */
      @Override public void registrationChangeFailed(int targetAccessTech, android.telephony.ims.ImsReasonInfo imsReasonInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(targetAccessTech);
          if ((imsReasonInfo!=null)) {
            _data.writeInt(1);
            imsReasonInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_registrationChangeFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registrationChangeFailed(targetAccessTech, imsReasonInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.ims.internal.IImsRegistrationListener sDefaultImpl;
    }
    static final int TRANSACTION_registrationConnected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_registrationProgressing = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_registrationConnectedWithRadioTech = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_registrationProgressingWithRadioTech = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_registrationDisconnected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_registrationResumed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_registrationSuspended = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_registrationServiceCapabilityChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_registrationFeatureCapabilityChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_voiceMessageCountUpdate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_registrationAssociatedUriChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_registrationChangeFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    public static boolean setDefaultImpl(com.android.ims.internal.IImsRegistrationListener impl) {
      // Only one user of this interface can use this function
      // at a time. This is a heuristic to detect if two different
      // users in the same process use this function.
      if (Stub.Proxy.sDefaultImpl != null) {
        throw new IllegalStateException("setDefaultImpl() called twice");
      }
      if (impl != null) {
        Stub.Proxy.sDefaultImpl = impl;
        return true;
      }
      return false;
    }
    public static com.android.ims.internal.IImsRegistrationListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Notifies the application when the device is connected to the IMS network.
       *
       * @deprecated see {@link registrationConnectedWithRadioTech}
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/IImsRegistrationListener.aidl:35:1:35:25")
  public void registrationConnected() throws android.os.RemoteException;
  /**
       * Notifies the application when the device is trying to connect the IMS network.
       *
       * @deprecated see {@link registrationProgressingWithRadioTech}
       */
  public void registrationProgressing() throws android.os.RemoteException;
  /**
       * Notifies the application when the device is connected to the IMS network.
       *
       * @param imsRadioTech the radio access technology. Valid values are {@code
       * RIL_RADIO_TECHNOLOGY_*} defined in {@link ServiceState}.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/IImsRegistrationListener.aidl:51:1:51:25")
  public void registrationConnectedWithRadioTech(int imsRadioTech) throws android.os.RemoteException;
  /**
       * Notifies the application when the device is trying to connect the IMS network.
       *
       * @param imsRadioTech the radio access technology. Valid values are {@code
       * RIL_RADIO_TECHNOLOGY_*} defined in {@link ServiceState}.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/IImsRegistrationListener.aidl:60:1:60:25")
  public void registrationProgressingWithRadioTech(int imsRadioTech) throws android.os.RemoteException;
  /**
       * Notifies the application when the device is disconnected from the IMS network.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/IImsRegistrationListener.aidl:67:1:67:25")
  public void registrationDisconnected(android.telephony.ims.ImsReasonInfo imsReasonInfo) throws android.os.RemoteException;
  /**
       * Notifies the application when its suspended IMS connection is resumed,
       * meaning the connection now allows throughput.
       */
  public void registrationResumed() throws android.os.RemoteException;
  /**
       * Notifies the application when its current IMS connection is suspended,
       * meaning there is no data throughput.
       */
  public void registrationSuspended() throws android.os.RemoteException;
  /**
       * Notifies the application when its current IMS connection is updated
       * since the service setting is changed or the service is added/removed.
       *
       * @param serviceClass a service class specified in {@link ImsServiceClass}
       * @param event an event type when this callback is called
       *    If {@code event} is 0, meaning the specified service is removed from the IMS connection.
       *    Else ({@code event} is 1), meaning the specified service is added to the IMS connection.
       */
  public void registrationServiceCapabilityChanged(int serviceClass, int event) throws android.os.RemoteException;
  /**
       * Notifies the application when features on a particular service enabled or
       * disabled successfully based on user preferences.
       *
       * @param serviceClass a service class specified in {@link ImsServiceClass}
       * @param enabledFeatures features enabled as defined in com.android.ims.ImsConfig#FeatureConstants.
       * @param disabledFeatures features disabled as defined in com.android.ims.ImsConfig#FeatureConstants.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/IImsRegistrationListener.aidl:101:1:101:25")
  public void registrationFeatureCapabilityChanged(int serviceClass, int[] enabledFeatures, int[] disabledFeatures) throws android.os.RemoteException;
  /**
       * Updates the application with the waiting voice message count.
       * @param count The number of waiting voice messages.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/IImsRegistrationListener.aidl:109:1:109:25")
  public void voiceMessageCountUpdate(int count) throws android.os.RemoteException;
  /**
       * Notifies the application when the list of URIs associated with IMS client is updated.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/IImsRegistrationListener.aidl:115:1:115:25")
  public void registrationAssociatedUriChanged(android.net.Uri[] uris) throws android.os.RemoteException;
  /**
       * Notifies the application when IMS registration attempt on a target
       * access tech fails.
       *
       * @param targetAccessTech Radio access technology on which the IMS registration was
       *         attempted.
       * @param imsReasonInfo Reason for the failure.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/IImsRegistrationListener.aidl:126:1:126:25")
  public void registrationChangeFailed(int targetAccessTech, android.telephony.ims.ImsReasonInfo imsReasonInfo) throws android.os.RemoteException;
}
