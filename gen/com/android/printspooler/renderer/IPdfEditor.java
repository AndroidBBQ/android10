/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.printspooler.renderer;
/**
 * Interface for communication with a remote pdf editor.
 */
public interface IPdfEditor extends android.os.IInterface
{
  /** Default implementation for IPdfEditor. */
  public static class Default implements com.android.printspooler.renderer.IPdfEditor
  {
    @Override public int openDocument(android.os.ParcelFileDescriptor source) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void removePages(android.print.PageRange[] pages) throws android.os.RemoteException
    {
    }
    @Override public void applyPrintAttributes(android.print.PrintAttributes attributes) throws android.os.RemoteException
    {
    }
    @Override public void write(android.os.ParcelFileDescriptor destination) throws android.os.RemoteException
    {
    }
    @Override public void closeDocument() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.printspooler.renderer.IPdfEditor
  {
    private static final java.lang.String DESCRIPTOR = "com.android.printspooler.renderer.IPdfEditor";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.printspooler.renderer.IPdfEditor interface,
     * generating a proxy if needed.
     */
    public static com.android.printspooler.renderer.IPdfEditor asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.printspooler.renderer.IPdfEditor))) {
        return ((com.android.printspooler.renderer.IPdfEditor)iin);
      }
      return new com.android.printspooler.renderer.IPdfEditor.Stub.Proxy(obj);
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
        case TRANSACTION_openDocument:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelFileDescriptor _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _result = this.openDocument(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_removePages:
        {
          data.enforceInterface(descriptor);
          android.print.PageRange[] _arg0;
          _arg0 = data.createTypedArray(android.print.PageRange.CREATOR);
          this.removePages(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_applyPrintAttributes:
        {
          data.enforceInterface(descriptor);
          android.print.PrintAttributes _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.print.PrintAttributes.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.applyPrintAttributes(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_write:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelFileDescriptor _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.write(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_closeDocument:
        {
          data.enforceInterface(descriptor);
          this.closeDocument();
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.printspooler.renderer.IPdfEditor
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
      @Override public int openDocument(android.os.ParcelFileDescriptor source) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((source!=null)) {
            _data.writeInt(1);
            source.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_openDocument, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().openDocument(source);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void removePages(android.print.PageRange[] pages) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedArray(pages, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removePages, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removePages(pages);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void applyPrintAttributes(android.print.PrintAttributes attributes) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((attributes!=null)) {
            _data.writeInt(1);
            attributes.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_applyPrintAttributes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().applyPrintAttributes(attributes);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void write(android.os.ParcelFileDescriptor destination) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((destination!=null)) {
            _data.writeInt(1);
            destination.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_write, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().write(destination);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void closeDocument() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_closeDocument, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().closeDocument();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.printspooler.renderer.IPdfEditor sDefaultImpl;
    }
    static final int TRANSACTION_openDocument = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_removePages = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_applyPrintAttributes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_write = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_closeDocument = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(com.android.printspooler.renderer.IPdfEditor impl) {
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
    public static com.android.printspooler.renderer.IPdfEditor getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public int openDocument(android.os.ParcelFileDescriptor source) throws android.os.RemoteException;
  public void removePages(android.print.PageRange[] pages) throws android.os.RemoteException;
  public void applyPrintAttributes(android.print.PrintAttributes attributes) throws android.os.RemoteException;
  public void write(android.os.ParcelFileDescriptor destination) throws android.os.RemoteException;
  public void closeDocument() throws android.os.RemoteException;
}
