/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.telecom;
/**
 * Internal remote interface for in-call services.
 *
 * @see android.telecom.InCallService
 *
 * {@hide}
 */
public interface IInCallService extends android.os.IInterface
{
  /** Default implementation for IInCallService. */
  public static class Default implements com.android.internal.telecom.IInCallService
  {
    @Override public void setInCallAdapter(com.android.internal.telecom.IInCallAdapter inCallAdapter) throws android.os.RemoteException
    {
    }
    @Override public void addCall(android.telecom.ParcelableCall call) throws android.os.RemoteException
    {
    }
    @Override public void updateCall(android.telecom.ParcelableCall call) throws android.os.RemoteException
    {
    }
    @Override public void setPostDial(java.lang.String callId, java.lang.String remaining) throws android.os.RemoteException
    {
    }
    @Override public void setPostDialWait(java.lang.String callId, java.lang.String remaining) throws android.os.RemoteException
    {
    }
    @Override public void onCallAudioStateChanged(android.telecom.CallAudioState callAudioState) throws android.os.RemoteException
    {
    }
    @Override public void bringToForeground(boolean showDialpad) throws android.os.RemoteException
    {
    }
    @Override public void onCanAddCallChanged(boolean canAddCall) throws android.os.RemoteException
    {
    }
    @Override public void silenceRinger() throws android.os.RemoteException
    {
    }
    @Override public void onConnectionEvent(java.lang.String callId, java.lang.String event, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    @Override public void onRttUpgradeRequest(java.lang.String callId, int id) throws android.os.RemoteException
    {
    }
    @Override public void onRttInitiationFailure(java.lang.String callId, int reason) throws android.os.RemoteException
    {
    }
    @Override public void onHandoverFailed(java.lang.String callId, int error) throws android.os.RemoteException
    {
    }
    @Override public void onHandoverComplete(java.lang.String callId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.telecom.IInCallService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.telecom.IInCallService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.telecom.IInCallService interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.telecom.IInCallService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.telecom.IInCallService))) {
        return ((com.android.internal.telecom.IInCallService)iin);
      }
      return new com.android.internal.telecom.IInCallService.Stub.Proxy(obj);
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
        case TRANSACTION_setInCallAdapter:
        {
          data.enforceInterface(descriptor);
          com.android.internal.telecom.IInCallAdapter _arg0;
          _arg0 = com.android.internal.telecom.IInCallAdapter.Stub.asInterface(data.readStrongBinder());
          this.setInCallAdapter(_arg0);
          return true;
        }
        case TRANSACTION_addCall:
        {
          data.enforceInterface(descriptor);
          android.telecom.ParcelableCall _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telecom.ParcelableCall.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.addCall(_arg0);
          return true;
        }
        case TRANSACTION_updateCall:
        {
          data.enforceInterface(descriptor);
          android.telecom.ParcelableCall _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telecom.ParcelableCall.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.updateCall(_arg0);
          return true;
        }
        case TRANSACTION_setPostDial:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.setPostDial(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setPostDialWait:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.setPostDialWait(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onCallAudioStateChanged:
        {
          data.enforceInterface(descriptor);
          android.telecom.CallAudioState _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telecom.CallAudioState.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onCallAudioStateChanged(_arg0);
          return true;
        }
        case TRANSACTION_bringToForeground:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.bringToForeground(_arg0);
          return true;
        }
        case TRANSACTION_onCanAddCallChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onCanAddCallChanged(_arg0);
          return true;
        }
        case TRANSACTION_silenceRinger:
        {
          data.enforceInterface(descriptor);
          this.silenceRinger();
          return true;
        }
        case TRANSACTION_onConnectionEvent:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.onConnectionEvent(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onRttUpgradeRequest:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.onRttUpgradeRequest(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onRttInitiationFailure:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.onRttInitiationFailure(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onHandoverFailed:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.onHandoverFailed(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onHandoverComplete:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.onHandoverComplete(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.telecom.IInCallService
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
      @Override public void setInCallAdapter(com.android.internal.telecom.IInCallAdapter inCallAdapter) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((inCallAdapter!=null))?(inCallAdapter.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setInCallAdapter, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setInCallAdapter(inCallAdapter);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void addCall(android.telecom.ParcelableCall call) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((call!=null)) {
            _data.writeInt(1);
            call.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_addCall, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addCall(call);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void updateCall(android.telecom.ParcelableCall call) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((call!=null)) {
            _data.writeInt(1);
            call.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateCall, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateCall(call);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setPostDial(java.lang.String callId, java.lang.String remaining) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeString(remaining);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPostDial, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPostDial(callId, remaining);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setPostDialWait(java.lang.String callId, java.lang.String remaining) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeString(remaining);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPostDialWait, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPostDialWait(callId, remaining);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onCallAudioStateChanged(android.telecom.CallAudioState callAudioState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((callAudioState!=null)) {
            _data.writeInt(1);
            callAudioState.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCallAudioStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCallAudioStateChanged(callAudioState);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void bringToForeground(boolean showDialpad) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((showDialpad)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_bringToForeground, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().bringToForeground(showDialpad);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onCanAddCallChanged(boolean canAddCall) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((canAddCall)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCanAddCallChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCanAddCallChanged(canAddCall);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void silenceRinger() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_silenceRinger, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().silenceRinger();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onConnectionEvent(java.lang.String callId, java.lang.String event, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeString(event);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onConnectionEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onConnectionEvent(callId, event, extras);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onRttUpgradeRequest(java.lang.String callId, int id) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeInt(id);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRttUpgradeRequest, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRttUpgradeRequest(callId, id);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onRttInitiationFailure(java.lang.String callId, int reason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeInt(reason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRttInitiationFailure, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRttInitiationFailure(callId, reason);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onHandoverFailed(java.lang.String callId, int error) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeInt(error);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onHandoverFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onHandoverFailed(callId, error);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onHandoverComplete(java.lang.String callId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onHandoverComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onHandoverComplete(callId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.telecom.IInCallService sDefaultImpl;
    }
    static final int TRANSACTION_setInCallAdapter = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_addCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_updateCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_setPostDial = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_setPostDialWait = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onCallAudioStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_bringToForeground = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_onCanAddCallChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_silenceRinger = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_onConnectionEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_onRttUpgradeRequest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_onRttInitiationFailure = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_onHandoverFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_onHandoverComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    public static boolean setDefaultImpl(com.android.internal.telecom.IInCallService impl) {
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
    public static com.android.internal.telecom.IInCallService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void setInCallAdapter(com.android.internal.telecom.IInCallAdapter inCallAdapter) throws android.os.RemoteException;
  public void addCall(android.telecom.ParcelableCall call) throws android.os.RemoteException;
  public void updateCall(android.telecom.ParcelableCall call) throws android.os.RemoteException;
  public void setPostDial(java.lang.String callId, java.lang.String remaining) throws android.os.RemoteException;
  public void setPostDialWait(java.lang.String callId, java.lang.String remaining) throws android.os.RemoteException;
  public void onCallAudioStateChanged(android.telecom.CallAudioState callAudioState) throws android.os.RemoteException;
  public void bringToForeground(boolean showDialpad) throws android.os.RemoteException;
  public void onCanAddCallChanged(boolean canAddCall) throws android.os.RemoteException;
  public void silenceRinger() throws android.os.RemoteException;
  public void onConnectionEvent(java.lang.String callId, java.lang.String event, android.os.Bundle extras) throws android.os.RemoteException;
  public void onRttUpgradeRequest(java.lang.String callId, int id) throws android.os.RemoteException;
  public void onRttInitiationFailure(java.lang.String callId, int reason) throws android.os.RemoteException;
  public void onHandoverFailed(java.lang.String callId, int error) throws android.os.RemoteException;
  public void onHandoverComplete(java.lang.String callId) throws android.os.RemoteException;
}
