/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view;
/**
 * Interface for implementing an filter which observes and
 * potentially transforms the input event stream in the system.
 *
 * @hide
 */
public interface IInputFilter extends android.os.IInterface
{
  /** Default implementation for IInputFilter. */
  public static class Default implements android.view.IInputFilter
  {
    @Override public void install(android.view.IInputFilterHost host) throws android.os.RemoteException
    {
    }
    @Override public void uninstall() throws android.os.RemoteException
    {
    }
    @Override public void filterInputEvent(android.view.InputEvent event, int policyFlags) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.IInputFilter
  {
    private static final java.lang.String DESCRIPTOR = "android.view.IInputFilter";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.IInputFilter interface,
     * generating a proxy if needed.
     */
    public static android.view.IInputFilter asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.IInputFilter))) {
        return ((android.view.IInputFilter)iin);
      }
      return new android.view.IInputFilter.Stub.Proxy(obj);
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
        case TRANSACTION_install:
        {
          data.enforceInterface(descriptor);
          android.view.IInputFilterHost _arg0;
          _arg0 = android.view.IInputFilterHost.Stub.asInterface(data.readStrongBinder());
          this.install(_arg0);
          return true;
        }
        case TRANSACTION_uninstall:
        {
          data.enforceInterface(descriptor);
          this.uninstall();
          return true;
        }
        case TRANSACTION_filterInputEvent:
        {
          data.enforceInterface(descriptor);
          android.view.InputEvent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.InputEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.filterInputEvent(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.view.IInputFilter
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
      @Override public void install(android.view.IInputFilterHost host) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((host!=null))?(host.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_install, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().install(host);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void uninstall() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_uninstall, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().uninstall();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void filterInputEvent(android.view.InputEvent event, int policyFlags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((event!=null)) {
            _data.writeInt(1);
            event.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(policyFlags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_filterInputEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().filterInputEvent(event, policyFlags);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.view.IInputFilter sDefaultImpl;
    }
    static final int TRANSACTION_install = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_uninstall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_filterInputEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.view.IInputFilter impl) {
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
    public static android.view.IInputFilter getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void install(android.view.IInputFilterHost host) throws android.os.RemoteException;
  public void uninstall() throws android.os.RemoteException;
  public void filterInputEvent(android.view.InputEvent event, int policyFlags) throws android.os.RemoteException;
}
