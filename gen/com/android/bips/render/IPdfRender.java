/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.bips.render;
/**
 * Defines a simple PDF rendering service to protect the main process from
 * crashes or security issues caused by unexpected PDF data.
 */
public interface IPdfRender extends android.os.IInterface
{
  /** Default implementation for IPdfRender. */
  public static class Default implements com.android.bips.render.IPdfRender
  {
    /**
         * Open a new document, returning the page count or 0 on error
         */
    @Override public int openDocument(android.os.ParcelFileDescriptor file) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Return open document's page size in fractional points (1/72") or null on error.
         */
    @Override public com.android.bips.jni.SizeD getPageSize(int page) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Render a page from the open document as a bitmap.
         *
         * @param destFile File to receive a PNG compressed bitmap corresponding to the specified
         *                 portion of the page
         * @param y y-offset from the page in pixels at the specified zoom factor
         * @param width full-page width of bitmap to render
         * @param height height of strip to render
         * @return output receiver for bitmap output
         */
    @Override public android.os.ParcelFileDescriptor renderPageStripe(int page, int y, int width, int height, double zoomFactor) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Release all internal resources related to the open document
         */
    @Override public void closeDocument() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.bips.render.IPdfRender
  {
    private static final java.lang.String DESCRIPTOR = "com.android.bips.render.IPdfRender";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.bips.render.IPdfRender interface,
     * generating a proxy if needed.
     */
    public static com.android.bips.render.IPdfRender asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.bips.render.IPdfRender))) {
        return ((com.android.bips.render.IPdfRender)iin);
      }
      return new com.android.bips.render.IPdfRender.Stub.Proxy(obj);
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
        case TRANSACTION_getPageSize:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          com.android.bips.jni.SizeD _result = this.getPageSize(_arg0);
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_renderPageStripe:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          double _arg4;
          _arg4 = data.readDouble();
          android.os.ParcelFileDescriptor _result = this.renderPageStripe(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_closeDocument:
        {
          data.enforceInterface(descriptor);
          this.closeDocument();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.bips.render.IPdfRender
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
           * Open a new document, returning the page count or 0 on error
           */
      @Override public int openDocument(android.os.ParcelFileDescriptor file) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((file!=null)) {
            _data.writeInt(1);
            file.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_openDocument, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().openDocument(file);
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
      /**
           * Return open document's page size in fractional points (1/72") or null on error.
           */
      @Override public com.android.bips.jni.SizeD getPageSize(int page) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.bips.jni.SizeD _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(page);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPageSize, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPageSize(page);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = com.android.bips.jni.SizeD.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Render a page from the open document as a bitmap.
           *
           * @param destFile File to receive a PNG compressed bitmap corresponding to the specified
           *                 portion of the page
           * @param y y-offset from the page in pixels at the specified zoom factor
           * @param width full-page width of bitmap to render
           * @param height height of strip to render
           * @return output receiver for bitmap output
           */
      @Override public android.os.ParcelFileDescriptor renderPageStripe(int page, int y, int width, int height, double zoomFactor) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.ParcelFileDescriptor _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(page);
          _data.writeInt(y);
          _data.writeInt(width);
          _data.writeInt(height);
          _data.writeDouble(zoomFactor);
          boolean _status = mRemote.transact(Stub.TRANSACTION_renderPageStripe, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().renderPageStripe(page, y, width, height, zoomFactor);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Release all internal resources related to the open document
           */
      @Override public void closeDocument() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_closeDocument, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().closeDocument();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.bips.render.IPdfRender sDefaultImpl;
    }
    static final int TRANSACTION_openDocument = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getPageSize = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_renderPageStripe = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_closeDocument = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(com.android.bips.render.IPdfRender impl) {
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
    public static com.android.bips.render.IPdfRender getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Open a new document, returning the page count or 0 on error
       */
  public int openDocument(android.os.ParcelFileDescriptor file) throws android.os.RemoteException;
  /**
       * Return open document's page size in fractional points (1/72") or null on error.
       */
  public com.android.bips.jni.SizeD getPageSize(int page) throws android.os.RemoteException;
  /**
       * Render a page from the open document as a bitmap.
       *
       * @param destFile File to receive a PNG compressed bitmap corresponding to the specified
       *                 portion of the page
       * @param y y-offset from the page in pixels at the specified zoom factor
       * @param width full-page width of bitmap to render
       * @param height height of strip to render
       * @return output receiver for bitmap output
       */
  public android.os.ParcelFileDescriptor renderPageStripe(int page, int y, int width, int height, double zoomFactor) throws android.os.RemoteException;
  /**
       * Release all internal resources related to the open document
       */
  public void closeDocument() throws android.os.RemoteException;
}
