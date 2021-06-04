/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.contentsuggestions;
/**
 * Interface from the system to an implementation of a content suggestions service.
 *
 * @hide
 */
public interface IContentSuggestionsService extends android.os.IInterface
{
  /** Default implementation for IContentSuggestionsService. */
  public static class Default implements android.service.contentsuggestions.IContentSuggestionsService
  {
    @Override public void provideContextImage(int taskId, android.graphics.GraphicBuffer contextImage, int colorSpaceId, android.os.Bundle imageContextRequestExtras) throws android.os.RemoteException
    {
    }
    @Override public void suggestContentSelections(android.app.contentsuggestions.SelectionsRequest request, android.app.contentsuggestions.ISelectionsCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void classifyContentSelections(android.app.contentsuggestions.ClassificationsRequest request, android.app.contentsuggestions.IClassificationsCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void notifyInteraction(java.lang.String requestId, android.os.Bundle interaction) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.contentsuggestions.IContentSuggestionsService
  {
    private static final java.lang.String DESCRIPTOR = "android.service.contentsuggestions.IContentSuggestionsService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.contentsuggestions.IContentSuggestionsService interface,
     * generating a proxy if needed.
     */
    public static android.service.contentsuggestions.IContentSuggestionsService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.contentsuggestions.IContentSuggestionsService))) {
        return ((android.service.contentsuggestions.IContentSuggestionsService)iin);
      }
      return new android.service.contentsuggestions.IContentSuggestionsService.Stub.Proxy(obj);
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
        case TRANSACTION_provideContextImage:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.graphics.GraphicBuffer _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.graphics.GraphicBuffer.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          android.os.Bundle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.provideContextImage(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_suggestContentSelections:
        {
          data.enforceInterface(descriptor);
          android.app.contentsuggestions.SelectionsRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.app.contentsuggestions.SelectionsRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.app.contentsuggestions.ISelectionsCallback _arg1;
          _arg1 = android.app.contentsuggestions.ISelectionsCallback.Stub.asInterface(data.readStrongBinder());
          this.suggestContentSelections(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_classifyContentSelections:
        {
          data.enforceInterface(descriptor);
          android.app.contentsuggestions.ClassificationsRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.app.contentsuggestions.ClassificationsRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.app.contentsuggestions.IClassificationsCallback _arg1;
          _arg1 = android.app.contentsuggestions.IClassificationsCallback.Stub.asInterface(data.readStrongBinder());
          this.classifyContentSelections(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_notifyInteraction:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.notifyInteraction(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.contentsuggestions.IContentSuggestionsService
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
      @Override public void provideContextImage(int taskId, android.graphics.GraphicBuffer contextImage, int colorSpaceId, android.os.Bundle imageContextRequestExtras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(taskId);
          if ((contextImage!=null)) {
            _data.writeInt(1);
            contextImage.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(colorSpaceId);
          if ((imageContextRequestExtras!=null)) {
            _data.writeInt(1);
            imageContextRequestExtras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_provideContextImage, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().provideContextImage(taskId, contextImage, colorSpaceId, imageContextRequestExtras);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void suggestContentSelections(android.app.contentsuggestions.SelectionsRequest request, android.app.contentsuggestions.ISelectionsCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_suggestContentSelections, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().suggestContentSelections(request, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void classifyContentSelections(android.app.contentsuggestions.ClassificationsRequest request, android.app.contentsuggestions.IClassificationsCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_classifyContentSelections, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().classifyContentSelections(request, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void notifyInteraction(java.lang.String requestId, android.os.Bundle interaction) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(requestId);
          if ((interaction!=null)) {
            _data.writeInt(1);
            interaction.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyInteraction, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyInteraction(requestId, interaction);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.contentsuggestions.IContentSuggestionsService sDefaultImpl;
    }
    static final int TRANSACTION_provideContextImage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_suggestContentSelections = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_classifyContentSelections = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_notifyInteraction = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.service.contentsuggestions.IContentSuggestionsService impl) {
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
    public static android.service.contentsuggestions.IContentSuggestionsService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void provideContextImage(int taskId, android.graphics.GraphicBuffer contextImage, int colorSpaceId, android.os.Bundle imageContextRequestExtras) throws android.os.RemoteException;
  public void suggestContentSelections(android.app.contentsuggestions.SelectionsRequest request, android.app.contentsuggestions.ISelectionsCallback callback) throws android.os.RemoteException;
  public void classifyContentSelections(android.app.contentsuggestions.ClassificationsRequest request, android.app.contentsuggestions.IClassificationsCallback callback) throws android.os.RemoteException;
  public void notifyInteraction(java.lang.String requestId, android.os.Bundle interaction) throws android.os.RemoteException;
}
