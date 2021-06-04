/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view.accessibility;
/**
 * Callback for specifying the result for an asynchronous request made
 * via calling a method on IAccessibilityInteractionConnectionCallback.
 *
 * @hide
 */
public interface IAccessibilityInteractionConnectionCallback extends android.os.IInterface
{
  /** Default implementation for IAccessibilityInteractionConnectionCallback. */
  public static class Default implements android.view.accessibility.IAccessibilityInteractionConnectionCallback
  {
    /**
         * Sets the result of an async request that returns an {@link AccessibilityNodeInfo}.
         *
         * @param infos The result {@link AccessibilityNodeInfo}.
         * @param interactionId The interaction id to match the result with the request.
         */
    @Override public void setFindAccessibilityNodeInfoResult(android.view.accessibility.AccessibilityNodeInfo info, int interactionId) throws android.os.RemoteException
    {
    }
    /**
         * Sets the result of an async request that returns {@link AccessibilityNodeInfo}s.
         *
         * @param infos The result {@link AccessibilityNodeInfo}s.
         * @param interactionId The interaction id to match the result with the request.
         */
    @Override public void setFindAccessibilityNodeInfosResult(java.util.List<android.view.accessibility.AccessibilityNodeInfo> infos, int interactionId) throws android.os.RemoteException
    {
    }
    /**
         * Sets the result of a request to perform an accessibility action.
         *
         * @param Whether the action was performed.
         * @param interactionId The interaction id to match the result with the request.
         */
    @Override public void setPerformAccessibilityActionResult(boolean succeeded, int interactionId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.accessibility.IAccessibilityInteractionConnectionCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.view.accessibility.IAccessibilityInteractionConnectionCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.accessibility.IAccessibilityInteractionConnectionCallback interface,
     * generating a proxy if needed.
     */
    public static android.view.accessibility.IAccessibilityInteractionConnectionCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.accessibility.IAccessibilityInteractionConnectionCallback))) {
        return ((android.view.accessibility.IAccessibilityInteractionConnectionCallback)iin);
      }
      return new android.view.accessibility.IAccessibilityInteractionConnectionCallback.Stub.Proxy(obj);
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
        case TRANSACTION_setFindAccessibilityNodeInfoResult:
        {
          data.enforceInterface(descriptor);
          android.view.accessibility.AccessibilityNodeInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.accessibility.AccessibilityNodeInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.setFindAccessibilityNodeInfoResult(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setFindAccessibilityNodeInfosResult:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.view.accessibility.AccessibilityNodeInfo> _arg0;
          _arg0 = data.createTypedArrayList(android.view.accessibility.AccessibilityNodeInfo.CREATOR);
          int _arg1;
          _arg1 = data.readInt();
          this.setFindAccessibilityNodeInfosResult(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setPerformAccessibilityActionResult:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          this.setPerformAccessibilityActionResult(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.view.accessibility.IAccessibilityInteractionConnectionCallback
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
           * Sets the result of an async request that returns an {@link AccessibilityNodeInfo}.
           *
           * @param infos The result {@link AccessibilityNodeInfo}.
           * @param interactionId The interaction id to match the result with the request.
           */
      @Override public void setFindAccessibilityNodeInfoResult(android.view.accessibility.AccessibilityNodeInfo info, int interactionId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((info!=null)) {
            _data.writeInt(1);
            info.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(interactionId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFindAccessibilityNodeInfoResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFindAccessibilityNodeInfoResult(info, interactionId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Sets the result of an async request that returns {@link AccessibilityNodeInfo}s.
           *
           * @param infos The result {@link AccessibilityNodeInfo}s.
           * @param interactionId The interaction id to match the result with the request.
           */
      @Override public void setFindAccessibilityNodeInfosResult(java.util.List<android.view.accessibility.AccessibilityNodeInfo> infos, int interactionId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(infos);
          _data.writeInt(interactionId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFindAccessibilityNodeInfosResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFindAccessibilityNodeInfosResult(infos, interactionId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Sets the result of a request to perform an accessibility action.
           *
           * @param Whether the action was performed.
           * @param interactionId The interaction id to match the result with the request.
           */
      @Override public void setPerformAccessibilityActionResult(boolean succeeded, int interactionId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((succeeded)?(1):(0)));
          _data.writeInt(interactionId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPerformAccessibilityActionResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPerformAccessibilityActionResult(succeeded, interactionId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.view.accessibility.IAccessibilityInteractionConnectionCallback sDefaultImpl;
    }
    static final int TRANSACTION_setFindAccessibilityNodeInfoResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_setFindAccessibilityNodeInfosResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_setPerformAccessibilityActionResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.view.accessibility.IAccessibilityInteractionConnectionCallback impl) {
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
    public static android.view.accessibility.IAccessibilityInteractionConnectionCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Sets the result of an async request that returns an {@link AccessibilityNodeInfo}.
       *
       * @param infos The result {@link AccessibilityNodeInfo}.
       * @param interactionId The interaction id to match the result with the request.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/accessibility/IAccessibilityInteractionConnectionCallback.aidl:36:1:36:25")
  public void setFindAccessibilityNodeInfoResult(android.view.accessibility.AccessibilityNodeInfo info, int interactionId) throws android.os.RemoteException;
  /**
       * Sets the result of an async request that returns {@link AccessibilityNodeInfo}s.
       *
       * @param infos The result {@link AccessibilityNodeInfo}s.
       * @param interactionId The interaction id to match the result with the request.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/accessibility/IAccessibilityInteractionConnectionCallback.aidl:45:1:45:25")
  public void setFindAccessibilityNodeInfosResult(java.util.List<android.view.accessibility.AccessibilityNodeInfo> infos, int interactionId) throws android.os.RemoteException;
  /**
       * Sets the result of a request to perform an accessibility action.
       *
       * @param Whether the action was performed.
       * @param interactionId The interaction id to match the result with the request.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/accessibility/IAccessibilityInteractionConnectionCallback.aidl:55:1:55:25")
  public void setPerformAccessibilityActionResult(boolean succeeded, int interactionId) throws android.os.RemoteException;
}
