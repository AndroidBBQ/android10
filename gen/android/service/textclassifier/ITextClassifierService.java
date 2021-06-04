/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.textclassifier;
/**
 * TextClassifierService binder interface.
 * See TextClassifier for interface documentation.
 * {@hide}
 */
public interface ITextClassifierService extends android.os.IInterface
{
  /** Default implementation for ITextClassifierService. */
  public static class Default implements android.service.textclassifier.ITextClassifierService
  {
    @Override public void onSuggestSelection(android.view.textclassifier.TextClassificationSessionId sessionId, android.view.textclassifier.TextSelection.Request request, android.service.textclassifier.ITextClassifierCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void onClassifyText(android.view.textclassifier.TextClassificationSessionId sessionId, android.view.textclassifier.TextClassification.Request request, android.service.textclassifier.ITextClassifierCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void onGenerateLinks(android.view.textclassifier.TextClassificationSessionId sessionId, android.view.textclassifier.TextLinks.Request request, android.service.textclassifier.ITextClassifierCallback callback) throws android.os.RemoteException
    {
    }
    // TODO: Remove

    @Override public void onSelectionEvent(android.view.textclassifier.TextClassificationSessionId sessionId, android.view.textclassifier.SelectionEvent event) throws android.os.RemoteException
    {
    }
    @Override public void onTextClassifierEvent(android.view.textclassifier.TextClassificationSessionId sessionId, android.view.textclassifier.TextClassifierEvent event) throws android.os.RemoteException
    {
    }
    @Override public void onCreateTextClassificationSession(android.view.textclassifier.TextClassificationContext context, android.view.textclassifier.TextClassificationSessionId sessionId) throws android.os.RemoteException
    {
    }
    @Override public void onDestroyTextClassificationSession(android.view.textclassifier.TextClassificationSessionId sessionId) throws android.os.RemoteException
    {
    }
    @Override public void onDetectLanguage(android.view.textclassifier.TextClassificationSessionId sessionId, android.view.textclassifier.TextLanguage.Request request, android.service.textclassifier.ITextClassifierCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void onSuggestConversationActions(android.view.textclassifier.TextClassificationSessionId sessionId, android.view.textclassifier.ConversationActions.Request request, android.service.textclassifier.ITextClassifierCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.textclassifier.ITextClassifierService
  {
    private static final java.lang.String DESCRIPTOR = "android.service.textclassifier.ITextClassifierService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.textclassifier.ITextClassifierService interface,
     * generating a proxy if needed.
     */
    public static android.service.textclassifier.ITextClassifierService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.textclassifier.ITextClassifierService))) {
        return ((android.service.textclassifier.ITextClassifierService)iin);
      }
      return new android.service.textclassifier.ITextClassifierService.Stub.Proxy(obj);
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
        case TRANSACTION_onSuggestSelection:
        {
          data.enforceInterface(descriptor);
          android.view.textclassifier.TextClassificationSessionId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.textclassifier.TextClassificationSessionId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.view.textclassifier.TextSelection.Request _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.view.textclassifier.TextSelection.Request.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.service.textclassifier.ITextClassifierCallback _arg2;
          _arg2 = android.service.textclassifier.ITextClassifierCallback.Stub.asInterface(data.readStrongBinder());
          this.onSuggestSelection(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onClassifyText:
        {
          data.enforceInterface(descriptor);
          android.view.textclassifier.TextClassificationSessionId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.textclassifier.TextClassificationSessionId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.view.textclassifier.TextClassification.Request _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.view.textclassifier.TextClassification.Request.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.service.textclassifier.ITextClassifierCallback _arg2;
          _arg2 = android.service.textclassifier.ITextClassifierCallback.Stub.asInterface(data.readStrongBinder());
          this.onClassifyText(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onGenerateLinks:
        {
          data.enforceInterface(descriptor);
          android.view.textclassifier.TextClassificationSessionId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.textclassifier.TextClassificationSessionId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.view.textclassifier.TextLinks.Request _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.view.textclassifier.TextLinks.Request.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.service.textclassifier.ITextClassifierCallback _arg2;
          _arg2 = android.service.textclassifier.ITextClassifierCallback.Stub.asInterface(data.readStrongBinder());
          this.onGenerateLinks(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onSelectionEvent:
        {
          data.enforceInterface(descriptor);
          android.view.textclassifier.TextClassificationSessionId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.textclassifier.TextClassificationSessionId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.view.textclassifier.SelectionEvent _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.view.textclassifier.SelectionEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.onSelectionEvent(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onTextClassifierEvent:
        {
          data.enforceInterface(descriptor);
          android.view.textclassifier.TextClassificationSessionId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.textclassifier.TextClassificationSessionId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.view.textclassifier.TextClassifierEvent _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.view.textclassifier.TextClassifierEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.onTextClassifierEvent(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onCreateTextClassificationSession:
        {
          data.enforceInterface(descriptor);
          android.view.textclassifier.TextClassificationContext _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.textclassifier.TextClassificationContext.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.view.textclassifier.TextClassificationSessionId _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.view.textclassifier.TextClassificationSessionId.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.onCreateTextClassificationSession(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onDestroyTextClassificationSession:
        {
          data.enforceInterface(descriptor);
          android.view.textclassifier.TextClassificationSessionId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.textclassifier.TextClassificationSessionId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onDestroyTextClassificationSession(_arg0);
          return true;
        }
        case TRANSACTION_onDetectLanguage:
        {
          data.enforceInterface(descriptor);
          android.view.textclassifier.TextClassificationSessionId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.textclassifier.TextClassificationSessionId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.view.textclassifier.TextLanguage.Request _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.view.textclassifier.TextLanguage.Request.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.service.textclassifier.ITextClassifierCallback _arg2;
          _arg2 = android.service.textclassifier.ITextClassifierCallback.Stub.asInterface(data.readStrongBinder());
          this.onDetectLanguage(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onSuggestConversationActions:
        {
          data.enforceInterface(descriptor);
          android.view.textclassifier.TextClassificationSessionId _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.textclassifier.TextClassificationSessionId.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.view.textclassifier.ConversationActions.Request _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.view.textclassifier.ConversationActions.Request.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.service.textclassifier.ITextClassifierCallback _arg2;
          _arg2 = android.service.textclassifier.ITextClassifierCallback.Stub.asInterface(data.readStrongBinder());
          this.onSuggestConversationActions(_arg0, _arg1, _arg2);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.textclassifier.ITextClassifierService
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
      @Override public void onSuggestSelection(android.view.textclassifier.TextClassificationSessionId sessionId, android.view.textclassifier.TextSelection.Request request, android.service.textclassifier.ITextClassifierCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sessionId!=null)) {
            _data.writeInt(1);
            sessionId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSuggestSelection, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSuggestSelection(sessionId, request, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onClassifyText(android.view.textclassifier.TextClassificationSessionId sessionId, android.view.textclassifier.TextClassification.Request request, android.service.textclassifier.ITextClassifierCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sessionId!=null)) {
            _data.writeInt(1);
            sessionId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onClassifyText, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onClassifyText(sessionId, request, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onGenerateLinks(android.view.textclassifier.TextClassificationSessionId sessionId, android.view.textclassifier.TextLinks.Request request, android.service.textclassifier.ITextClassifierCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sessionId!=null)) {
            _data.writeInt(1);
            sessionId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGenerateLinks, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGenerateLinks(sessionId, request, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // TODO: Remove

      @Override public void onSelectionEvent(android.view.textclassifier.TextClassificationSessionId sessionId, android.view.textclassifier.SelectionEvent event) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sessionId!=null)) {
            _data.writeInt(1);
            sessionId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((event!=null)) {
            _data.writeInt(1);
            event.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSelectionEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSelectionEvent(sessionId, event);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onTextClassifierEvent(android.view.textclassifier.TextClassificationSessionId sessionId, android.view.textclassifier.TextClassifierEvent event) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sessionId!=null)) {
            _data.writeInt(1);
            sessionId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((event!=null)) {
            _data.writeInt(1);
            event.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onTextClassifierEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onTextClassifierEvent(sessionId, event);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onCreateTextClassificationSession(android.view.textclassifier.TextClassificationContext context, android.view.textclassifier.TextClassificationSessionId sessionId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((context!=null)) {
            _data.writeInt(1);
            context.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((sessionId!=null)) {
            _data.writeInt(1);
            sessionId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCreateTextClassificationSession, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCreateTextClassificationSession(context, sessionId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDestroyTextClassificationSession(android.view.textclassifier.TextClassificationSessionId sessionId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sessionId!=null)) {
            _data.writeInt(1);
            sessionId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDestroyTextClassificationSession, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDestroyTextClassificationSession(sessionId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDetectLanguage(android.view.textclassifier.TextClassificationSessionId sessionId, android.view.textclassifier.TextLanguage.Request request, android.service.textclassifier.ITextClassifierCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sessionId!=null)) {
            _data.writeInt(1);
            sessionId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDetectLanguage, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDetectLanguage(sessionId, request, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSuggestConversationActions(android.view.textclassifier.TextClassificationSessionId sessionId, android.view.textclassifier.ConversationActions.Request request, android.service.textclassifier.ITextClassifierCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sessionId!=null)) {
            _data.writeInt(1);
            sessionId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSuggestConversationActions, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSuggestConversationActions(sessionId, request, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.textclassifier.ITextClassifierService sDefaultImpl;
    }
    static final int TRANSACTION_onSuggestSelection = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onClassifyText = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onGenerateLinks = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onSelectionEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onTextClassifierEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onCreateTextClassificationSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_onDestroyTextClassificationSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_onDetectLanguage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_onSuggestConversationActions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    public static boolean setDefaultImpl(android.service.textclassifier.ITextClassifierService impl) {
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
    public static android.service.textclassifier.ITextClassifierService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onSuggestSelection(android.view.textclassifier.TextClassificationSessionId sessionId, android.view.textclassifier.TextSelection.Request request, android.service.textclassifier.ITextClassifierCallback callback) throws android.os.RemoteException;
  public void onClassifyText(android.view.textclassifier.TextClassificationSessionId sessionId, android.view.textclassifier.TextClassification.Request request, android.service.textclassifier.ITextClassifierCallback callback) throws android.os.RemoteException;
  public void onGenerateLinks(android.view.textclassifier.TextClassificationSessionId sessionId, android.view.textclassifier.TextLinks.Request request, android.service.textclassifier.ITextClassifierCallback callback) throws android.os.RemoteException;
  // TODO: Remove

  public void onSelectionEvent(android.view.textclassifier.TextClassificationSessionId sessionId, android.view.textclassifier.SelectionEvent event) throws android.os.RemoteException;
  public void onTextClassifierEvent(android.view.textclassifier.TextClassificationSessionId sessionId, android.view.textclassifier.TextClassifierEvent event) throws android.os.RemoteException;
  public void onCreateTextClassificationSession(android.view.textclassifier.TextClassificationContext context, android.view.textclassifier.TextClassificationSessionId sessionId) throws android.os.RemoteException;
  public void onDestroyTextClassificationSession(android.view.textclassifier.TextClassificationSessionId sessionId) throws android.os.RemoteException;
  public void onDetectLanguage(android.view.textclassifier.TextClassificationSessionId sessionId, android.view.textclassifier.TextLanguage.Request request, android.service.textclassifier.ITextClassifierCallback callback) throws android.os.RemoteException;
  public void onSuggestConversationActions(android.view.textclassifier.TextClassificationSessionId sessionId, android.view.textclassifier.ConversationActions.Request request, android.service.textclassifier.ITextClassifierCallback callback) throws android.os.RemoteException;
}
