/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net;
/** {@hide} */
public interface INetworkPolicyListener extends android.os.IInterface
{
  /** Default implementation for INetworkPolicyListener. */
  public static class Default implements android.net.INetworkPolicyListener
  {
    @Override public void onUidRulesChanged(int uid, int uidRules) throws android.os.RemoteException
    {
    }
    @Override public void onMeteredIfacesChanged(java.lang.String[] meteredIfaces) throws android.os.RemoteException
    {
    }
    @Override public void onRestrictBackgroundChanged(boolean restrictBackground) throws android.os.RemoteException
    {
    }
    @Override public void onUidPoliciesChanged(int uid, int uidPolicies) throws android.os.RemoteException
    {
    }
    @Override public void onSubscriptionOverride(int subId, int overrideMask, int overrideValue) throws android.os.RemoteException
    {
    }
    @Override public void onSubscriptionPlansChanged(int subId, android.telephony.SubscriptionPlan[] plans) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.INetworkPolicyListener
  {
    private static final java.lang.String DESCRIPTOR = "android.net.INetworkPolicyListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.INetworkPolicyListener interface,
     * generating a proxy if needed.
     */
    public static android.net.INetworkPolicyListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.INetworkPolicyListener))) {
        return ((android.net.INetworkPolicyListener)iin);
      }
      return new android.net.INetworkPolicyListener.Stub.Proxy(obj);
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
        case TRANSACTION_onUidRulesChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.onUidRulesChanged(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onMeteredIfacesChanged:
        {
          data.enforceInterface(descriptor);
          java.lang.String[] _arg0;
          _arg0 = data.createStringArray();
          this.onMeteredIfacesChanged(_arg0);
          return true;
        }
        case TRANSACTION_onRestrictBackgroundChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onRestrictBackgroundChanged(_arg0);
          return true;
        }
        case TRANSACTION_onUidPoliciesChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.onUidPoliciesChanged(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onSubscriptionOverride:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.onSubscriptionOverride(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onSubscriptionPlansChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.SubscriptionPlan[] _arg1;
          _arg1 = data.createTypedArray(android.telephony.SubscriptionPlan.CREATOR);
          this.onSubscriptionPlansChanged(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.INetworkPolicyListener
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
      @Override public void onUidRulesChanged(int uid, int uidRules) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeInt(uidRules);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onUidRulesChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onUidRulesChanged(uid, uidRules);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onMeteredIfacesChanged(java.lang.String[] meteredIfaces) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStringArray(meteredIfaces);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onMeteredIfacesChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onMeteredIfacesChanged(meteredIfaces);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onRestrictBackgroundChanged(boolean restrictBackground) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((restrictBackground)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRestrictBackgroundChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRestrictBackgroundChanged(restrictBackground);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onUidPoliciesChanged(int uid, int uidPolicies) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeInt(uidPolicies);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onUidPoliciesChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onUidPoliciesChanged(uid, uidPolicies);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSubscriptionOverride(int subId, int overrideMask, int overrideValue) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(overrideMask);
          _data.writeInt(overrideValue);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSubscriptionOverride, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSubscriptionOverride(subId, overrideMask, overrideValue);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSubscriptionPlansChanged(int subId, android.telephony.SubscriptionPlan[] plans) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeTypedArray(plans, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSubscriptionPlansChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSubscriptionPlansChanged(subId, plans);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.INetworkPolicyListener sDefaultImpl;
    }
    static final int TRANSACTION_onUidRulesChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onMeteredIfacesChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onRestrictBackgroundChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onUidPoliciesChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onSubscriptionOverride = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onSubscriptionPlansChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(android.net.INetworkPolicyListener impl) {
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
    public static android.net.INetworkPolicyListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onUidRulesChanged(int uid, int uidRules) throws android.os.RemoteException;
  public void onMeteredIfacesChanged(java.lang.String[] meteredIfaces) throws android.os.RemoteException;
  public void onRestrictBackgroundChanged(boolean restrictBackground) throws android.os.RemoteException;
  public void onUidPoliciesChanged(int uid, int uidPolicies) throws android.os.RemoteException;
  public void onSubscriptionOverride(int subId, int overrideMask, int overrideValue) throws android.os.RemoteException;
  public void onSubscriptionPlansChanged(int subId, android.telephony.SubscriptionPlan[] plans) throws android.os.RemoteException;
}
