/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.dialer.simulator.service;
public interface ISimulatorService extends android.os.IInterface
{
  /** Default implementation for ISimulatorService. */
  public static class Default implements com.android.dialer.simulator.service.ISimulatorService
  {
    /**
      * Makes an incoming call by simulator api.
      * @param callerId is the number showing on incall UI.
      * @param presentation is one of types of a call e.g. Payphone, Restricted, etc.. check
      * {@link TelecomManager} for more information.
      * */
    @Override public void makeIncomingCall(java.lang.String callerId, int presentation) throws android.os.RemoteException
    {
    }
    /**
      * Makes an incoming call.
      * @param callerId the number showing on incall UI.
      * @param presentation one of types of a call e.g. Payphone, Restricted, etc.. check
      * {@link TelecomManager} for more information.
      * */
    @Override public void makeOutgoingCall(java.lang.String callerId, int presentation) throws android.os.RemoteException
    {
    }
    /**
      * Makes an incoming enriched call.
      * Note: simulator mode should be enabled first.
      * */
    @Override public void makeIncomingEnrichedCall() throws android.os.RemoteException
    {
    }
    /**
      * Makes an outgoing enriched call.
      * Note: simulator mode should be enabled first.
      * */
    @Override public void makeOutgoingEnrichedCall() throws android.os.RemoteException
    {
    }
    /**
      * Populate missed call logs.
      * @param num the number of missed call to make with this api.
      * */
    @Override public void populateMissedCall(int num) throws android.os.RemoteException
    {
    }
    /** Populate contacts database to get contacts, call logs, voicemails, etc.. */
    @Override public void populateDataBase() throws android.os.RemoteException
    {
    }
    /** Clean contacts database to clean all exsting contacts, call logs. voicemails, etc.. */
    @Override public void cleanDataBase() throws android.os.RemoteException
    {
    }
    /**
      * Enable simulator mode. After entering simulator mode, all calls made by dialer will be handled
      * by simulator connection service, meaning users can directly make fake calls through simulator.
      * It is also a prerequisite to make an enriched call.
      * */
    @Override public void enableSimulatorMode() throws android.os.RemoteException
    {
    }
    /** Disable simulator mode to use system connection service. */
    @Override public void disableSimulatorMode() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.dialer.simulator.service.ISimulatorService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.dialer.simulator.service.ISimulatorService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.dialer.simulator.service.ISimulatorService interface,
     * generating a proxy if needed.
     */
    public static com.android.dialer.simulator.service.ISimulatorService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.dialer.simulator.service.ISimulatorService))) {
        return ((com.android.dialer.simulator.service.ISimulatorService)iin);
      }
      return new com.android.dialer.simulator.service.ISimulatorService.Stub.Proxy(obj);
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
        case TRANSACTION_makeIncomingCall:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.makeIncomingCall(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_makeOutgoingCall:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.makeOutgoingCall(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_makeIncomingEnrichedCall:
        {
          data.enforceInterface(descriptor);
          this.makeIncomingEnrichedCall();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_makeOutgoingEnrichedCall:
        {
          data.enforceInterface(descriptor);
          this.makeOutgoingEnrichedCall();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_populateMissedCall:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.populateMissedCall(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_populateDataBase:
        {
          data.enforceInterface(descriptor);
          this.populateDataBase();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_cleanDataBase:
        {
          data.enforceInterface(descriptor);
          this.cleanDataBase();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_enableSimulatorMode:
        {
          data.enforceInterface(descriptor);
          this.enableSimulatorMode();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_disableSimulatorMode:
        {
          data.enforceInterface(descriptor);
          this.disableSimulatorMode();
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.dialer.simulator.service.ISimulatorService
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
        * Makes an incoming call by simulator api.
        * @param callerId is the number showing on incall UI.
        * @param presentation is one of types of a call e.g. Payphone, Restricted, etc.. check
        * {@link TelecomManager} for more information.
        * */
      @Override public void makeIncomingCall(java.lang.String callerId, int presentation) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callerId);
          _data.writeInt(presentation);
          boolean _status = mRemote.transact(Stub.TRANSACTION_makeIncomingCall, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().makeIncomingCall(callerId, presentation);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
        * Makes an incoming call.
        * @param callerId the number showing on incall UI.
        * @param presentation one of types of a call e.g. Payphone, Restricted, etc.. check
        * {@link TelecomManager} for more information.
        * */
      @Override public void makeOutgoingCall(java.lang.String callerId, int presentation) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callerId);
          _data.writeInt(presentation);
          boolean _status = mRemote.transact(Stub.TRANSACTION_makeOutgoingCall, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().makeOutgoingCall(callerId, presentation);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
        * Makes an incoming enriched call.
        * Note: simulator mode should be enabled first.
        * */
      @Override public void makeIncomingEnrichedCall() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_makeIncomingEnrichedCall, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().makeIncomingEnrichedCall();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
        * Makes an outgoing enriched call.
        * Note: simulator mode should be enabled first.
        * */
      @Override public void makeOutgoingEnrichedCall() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_makeOutgoingEnrichedCall, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().makeOutgoingEnrichedCall();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
        * Populate missed call logs.
        * @param num the number of missed call to make with this api.
        * */
      @Override public void populateMissedCall(int num) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(num);
          boolean _status = mRemote.transact(Stub.TRANSACTION_populateMissedCall, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().populateMissedCall(num);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Populate contacts database to get contacts, call logs, voicemails, etc.. */
      @Override public void populateDataBase() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_populateDataBase, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().populateDataBase();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Clean contacts database to clean all exsting contacts, call logs. voicemails, etc.. */
      @Override public void cleanDataBase() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_cleanDataBase, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cleanDataBase();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
        * Enable simulator mode. After entering simulator mode, all calls made by dialer will be handled
        * by simulator connection service, meaning users can directly make fake calls through simulator.
        * It is also a prerequisite to make an enriched call.
        * */
      @Override public void enableSimulatorMode() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableSimulatorMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enableSimulatorMode();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Disable simulator mode to use system connection service. */
      @Override public void disableSimulatorMode() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableSimulatorMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableSimulatorMode();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.dialer.simulator.service.ISimulatorService sDefaultImpl;
    }
    static final int TRANSACTION_makeIncomingCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_makeOutgoingCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_makeIncomingEnrichedCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_makeOutgoingEnrichedCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_populateMissedCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_populateDataBase = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_cleanDataBase = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_enableSimulatorMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_disableSimulatorMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    public static boolean setDefaultImpl(com.android.dialer.simulator.service.ISimulatorService impl) {
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
    public static com.android.dialer.simulator.service.ISimulatorService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
    * Makes an incoming call by simulator api.
    * @param callerId is the number showing on incall UI.
    * @param presentation is one of types of a call e.g. Payphone, Restricted, etc.. check
    * {@link TelecomManager} for more information.
    * */
  public void makeIncomingCall(java.lang.String callerId, int presentation) throws android.os.RemoteException;
  /**
    * Makes an incoming call.
    * @param callerId the number showing on incall UI.
    * @param presentation one of types of a call e.g. Payphone, Restricted, etc.. check
    * {@link TelecomManager} for more information.
    * */
  public void makeOutgoingCall(java.lang.String callerId, int presentation) throws android.os.RemoteException;
  /**
    * Makes an incoming enriched call.
    * Note: simulator mode should be enabled first.
    * */
  public void makeIncomingEnrichedCall() throws android.os.RemoteException;
  /**
    * Makes an outgoing enriched call.
    * Note: simulator mode should be enabled first.
    * */
  public void makeOutgoingEnrichedCall() throws android.os.RemoteException;
  /**
    * Populate missed call logs.
    * @param num the number of missed call to make with this api.
    * */
  public void populateMissedCall(int num) throws android.os.RemoteException;
  /** Populate contacts database to get contacts, call logs, voicemails, etc.. */
  public void populateDataBase() throws android.os.RemoteException;
  /** Clean contacts database to clean all exsting contacts, call logs. voicemails, etc.. */
  public void cleanDataBase() throws android.os.RemoteException;
  /**
    * Enable simulator mode. After entering simulator mode, all calls made by dialer will be handled
    * by simulator connection service, meaning users can directly make fake calls through simulator.
    * It is also a prerequisite to make an enriched call.
    * */
  public void enableSimulatorMode() throws android.os.RemoteException;
  /** Disable simulator mode to use system connection service. */
  public void disableSimulatorMode() throws android.os.RemoteException;
}
