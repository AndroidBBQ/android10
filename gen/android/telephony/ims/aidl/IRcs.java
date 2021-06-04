/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony.ims.aidl;
/**
 * RPC definition between RCS storage APIs and phone process.
 * {@hide}
 */
public interface IRcs extends android.os.IInterface
{
  /** Default implementation for IRcs. */
  public static class Default implements android.telephony.ims.aidl.IRcs
  {
    /////////////////////////
    // RcsMessageStore APIs
    /////////////////////////

    @Override public android.telephony.ims.RcsThreadQueryResultParcelable getRcsThreads(android.telephony.ims.RcsThreadQueryParams queryParams, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.telephony.ims.RcsThreadQueryResultParcelable getRcsThreadsWithToken(android.telephony.ims.RcsQueryContinuationToken continuationToken, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.telephony.ims.RcsParticipantQueryResultParcelable getParticipants(android.telephony.ims.RcsParticipantQueryParams queryParams, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.telephony.ims.RcsParticipantQueryResultParcelable getParticipantsWithToken(android.telephony.ims.RcsQueryContinuationToken continuationToken, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.telephony.ims.RcsMessageQueryResultParcelable getMessages(android.telephony.ims.RcsMessageQueryParams queryParams, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.telephony.ims.RcsMessageQueryResultParcelable getMessagesWithToken(android.telephony.ims.RcsQueryContinuationToken continuationToken, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.telephony.ims.RcsEventQueryResultDescriptor getEvents(android.telephony.ims.RcsEventQueryParams queryParams, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.telephony.ims.RcsEventQueryResultDescriptor getEventsWithToken(android.telephony.ims.RcsQueryContinuationToken continuationToken, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    // returns true if the thread was successfully deleted

    @Override public boolean deleteThread(int threadId, int threadType, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    // Creates an Rcs1To1Thread and returns its row ID

    @Override public int createRcs1To1Thread(int participantId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    // Creates an RcsGroupThread and returns its row ID

    @Override public int createGroupThread(int[] participantIds, java.lang.String groupName, android.net.Uri groupIcon, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /////////////////////////
    // RcsThread APIs
    /////////////////////////
    // Creates a new RcsIncomingMessage on the given thread and returns its row ID

    @Override public int addIncomingMessage(int rcsThreadId, android.telephony.ims.RcsIncomingMessageCreationParams rcsIncomingMessageCreationParams, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    // Creates a new RcsOutgoingMessage on the given thread and returns its row ID

    @Override public int addOutgoingMessage(int rcsThreadId, android.telephony.ims.RcsOutgoingMessageCreationParams rcsOutgoingMessageCreationParams, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    // TODO: modify RcsProvider URI's to allow deleting a message without specifying its thread

    @Override public void deleteMessage(int rcsMessageId, boolean isIncoming, int rcsThreadId, boolean isGroup, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public android.telephony.ims.RcsMessageSnippet getMessageSnippet(int rcsThreadId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /////////////////////////
    // Rcs1To1Thread APIs
    /////////////////////////

    @Override public void set1To1ThreadFallbackThreadId(int rcsThreadId, long fallbackId, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public long get1To1ThreadFallbackThreadId(int rcsThreadId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0L;
    }
    @Override public int get1To1ThreadOtherParticipantId(int rcsThreadId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /////////////////////////
    // RcsGroupThread APIs
    /////////////////////////

    @Override public void setGroupThreadName(int rcsThreadId, java.lang.String groupName, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String getGroupThreadName(int rcsThreadId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setGroupThreadIcon(int rcsThreadId, android.net.Uri groupIcon, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public android.net.Uri getGroupThreadIcon(int rcsThreadId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setGroupThreadOwner(int rcsThreadId, int participantId, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public int getGroupThreadOwner(int rcsThreadId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setGroupThreadConferenceUri(int rcsThreadId, android.net.Uri conferenceUri, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public android.net.Uri getGroupThreadConferenceUri(int rcsThreadId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void addParticipantToGroupThread(int rcsThreadId, int participantId, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public void removeParticipantFromGroupThread(int rcsThreadId, int participantId, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    /////////////////////////
    // RcsParticipant APIs
    /////////////////////////
    // Creates a new RcsParticipant and returns its rowId

    @Override public int createRcsParticipant(java.lang.String canonicalAddress, java.lang.String alias, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public java.lang.String getRcsParticipantCanonicalAddress(int participantId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String getRcsParticipantAlias(int participantId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setRcsParticipantAlias(int id, java.lang.String alias, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String getRcsParticipantContactId(int participantId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setRcsParticipantContactId(int participantId, java.lang.String contactId, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    /////////////////////////
    // RcsMessage APIs
    /////////////////////////

    @Override public void setMessageSubId(int messageId, boolean isIncoming, int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public int getMessageSubId(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setMessageStatus(int messageId, boolean isIncoming, int status, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public int getMessageStatus(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setMessageOriginationTimestamp(int messageId, boolean isIncoming, long originationTimestamp, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public long getMessageOriginationTimestamp(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0L;
    }
    @Override public void setGlobalMessageIdForMessage(int messageId, boolean isIncoming, java.lang.String globalId, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String getGlobalMessageIdForMessage(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setMessageArrivalTimestamp(int messageId, boolean isIncoming, long arrivalTimestamp, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public long getMessageArrivalTimestamp(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0L;
    }
    @Override public void setMessageSeenTimestamp(int messageId, boolean isIncoming, long seenTimestamp, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public long getMessageSeenTimestamp(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0L;
    }
    @Override public void setTextForMessage(int messageId, boolean isIncoming, java.lang.String text, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String getTextForMessage(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setLatitudeForMessage(int messageId, boolean isIncoming, double latitude, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public double getLatitudeForMessage(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0.0d;
    }
    @Override public void setLongitudeForMessage(int messageId, boolean isIncoming, double longitude, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public double getLongitudeForMessage(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0.0d;
    }
    // Returns the ID's of the file transfers attached to the given message

    @Override public int[] getFileTransfersAttachedToMessage(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public int getSenderParticipant(int messageId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /////////////////////////
    // RcsOutgoingMessageDelivery APIs
    /////////////////////////
    // Returns the participant ID's that this message is intended to be delivered to

    @Override public int[] getMessageRecipients(int messageId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public long getOutgoingDeliveryDeliveredTimestamp(int messageId, int participantId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0L;
    }
    @Override public void setOutgoingDeliveryDeliveredTimestamp(int messageId, int participantId, long deliveredTimestamp, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public long getOutgoingDeliverySeenTimestamp(int messageId, int participantId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0L;
    }
    @Override public void setOutgoingDeliverySeenTimestamp(int messageId, int participantId, long seenTimestamp, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public int getOutgoingDeliveryStatus(int messageId, int participantId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setOutgoingDeliveryStatus(int messageId, int participantId, int status, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    /////////////////////////
    // RcsFileTransferPart APIs
    /////////////////////////
    // Performs the initial write to storage and returns the row ID.

    @Override public int storeFileTransfer(int messageId, boolean isIncoming, android.telephony.ims.RcsFileTransferCreationParams fileTransferCreationParams, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void deleteFileTransfer(int partId, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public void setFileTransferSessionId(int partId, java.lang.String sessionId, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String getFileTransferSessionId(int partId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setFileTransferContentUri(int partId, android.net.Uri contentUri, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public android.net.Uri getFileTransferContentUri(int partId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setFileTransferContentType(int partId, java.lang.String contentType, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String getFileTransferContentType(int partId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setFileTransferFileSize(int partId, long fileSize, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public long getFileTransferFileSize(int partId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0L;
    }
    @Override public void setFileTransferTransferOffset(int partId, long transferOffset, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public long getFileTransferTransferOffset(int partId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0L;
    }
    @Override public void setFileTransferStatus(int partId, int transferStatus, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public int getFileTransferStatus(int partId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setFileTransferWidth(int partId, int width, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public int getFileTransferWidth(int partId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setFileTransferHeight(int partId, int height, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public int getFileTransferHeight(int partId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setFileTransferLength(int partId, long length, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public long getFileTransferLength(int partId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0L;
    }
    @Override public void setFileTransferPreviewUri(int partId, android.net.Uri uri, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public android.net.Uri getFileTransferPreviewUri(int partId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setFileTransferPreviewType(int partId, java.lang.String type, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String getFileTransferPreviewType(int partId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /////////////////////////
    // RcsEvent APIs
    /////////////////////////

    @Override public int createGroupThreadNameChangedEvent(long timestamp, int threadId, int originationParticipantId, java.lang.String newName, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int createGroupThreadIconChangedEvent(long timestamp, int threadId, int originationParticipantId, android.net.Uri newIcon, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int createGroupThreadParticipantJoinedEvent(long timestamp, int threadId, int originationParticipantId, int participantId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int createGroupThreadParticipantLeftEvent(long timestamp, int threadId, int originationParticipantId, int participantId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int createParticipantAliasChangedEvent(long timestamp, int participantId, java.lang.String newAlias, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.ims.aidl.IRcs
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.ims.aidl.IRcs";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.ims.aidl.IRcs interface,
     * generating a proxy if needed.
     */
    public static android.telephony.ims.aidl.IRcs asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.ims.aidl.IRcs))) {
        return ((android.telephony.ims.aidl.IRcs)iin);
      }
      return new android.telephony.ims.aidl.IRcs.Stub.Proxy(obj);
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
        case TRANSACTION_getRcsThreads:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.RcsThreadQueryParams _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.RcsThreadQueryParams.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.telephony.ims.RcsThreadQueryResultParcelable _result = this.getRcsThreads(_arg0, _arg1);
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
        case TRANSACTION_getRcsThreadsWithToken:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.RcsQueryContinuationToken _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.RcsQueryContinuationToken.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.telephony.ims.RcsThreadQueryResultParcelable _result = this.getRcsThreadsWithToken(_arg0, _arg1);
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
        case TRANSACTION_getParticipants:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.RcsParticipantQueryParams _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.RcsParticipantQueryParams.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.telephony.ims.RcsParticipantQueryResultParcelable _result = this.getParticipants(_arg0, _arg1);
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
        case TRANSACTION_getParticipantsWithToken:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.RcsQueryContinuationToken _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.RcsQueryContinuationToken.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.telephony.ims.RcsParticipantQueryResultParcelable _result = this.getParticipantsWithToken(_arg0, _arg1);
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
        case TRANSACTION_getMessages:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.RcsMessageQueryParams _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.RcsMessageQueryParams.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.telephony.ims.RcsMessageQueryResultParcelable _result = this.getMessages(_arg0, _arg1);
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
        case TRANSACTION_getMessagesWithToken:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.RcsQueryContinuationToken _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.RcsQueryContinuationToken.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.telephony.ims.RcsMessageQueryResultParcelable _result = this.getMessagesWithToken(_arg0, _arg1);
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
        case TRANSACTION_getEvents:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.RcsEventQueryParams _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.RcsEventQueryParams.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.telephony.ims.RcsEventQueryResultDescriptor _result = this.getEvents(_arg0, _arg1);
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
        case TRANSACTION_getEventsWithToken:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.RcsQueryContinuationToken _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.RcsQueryContinuationToken.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.telephony.ims.RcsEventQueryResultDescriptor _result = this.getEventsWithToken(_arg0, _arg1);
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
        case TRANSACTION_deleteThread:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          boolean _result = this.deleteThread(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_createRcs1To1Thread:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.createRcs1To1Thread(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_createGroupThread:
        {
          data.enforceInterface(descriptor);
          int[] _arg0;
          _arg0 = data.createIntArray();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.net.Uri _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          java.lang.String _arg3;
          _arg3 = data.readString();
          int _result = this.createGroupThread(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_addIncomingMessage:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.ims.RcsIncomingMessageCreationParams _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telephony.ims.RcsIncomingMessageCreationParams.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _result = this.addIncomingMessage(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_addOutgoingMessage:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.ims.RcsOutgoingMessageCreationParams _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telephony.ims.RcsOutgoingMessageCreationParams.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _result = this.addOutgoingMessage(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_deleteMessage:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          int _arg2;
          _arg2 = data.readInt();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          java.lang.String _arg4;
          _arg4 = data.readString();
          this.deleteMessage(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getMessageSnippet:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.telephony.ims.RcsMessageSnippet _result = this.getMessageSnippet(_arg0, _arg1);
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
        case TRANSACTION_set1To1ThreadFallbackThreadId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _arg1;
          _arg1 = data.readLong();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.set1To1ThreadFallbackThreadId(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_get1To1ThreadFallbackThreadId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          long _result = this.get1To1ThreadFallbackThreadId(_arg0, _arg1);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_get1To1ThreadOtherParticipantId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.get1To1ThreadOtherParticipantId(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setGroupThreadName:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setGroupThreadName(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getGroupThreadName:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _result = this.getGroupThreadName(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_setGroupThreadIcon:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.net.Uri _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setGroupThreadIcon(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getGroupThreadIcon:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.net.Uri _result = this.getGroupThreadIcon(_arg0, _arg1);
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
        case TRANSACTION_setGroupThreadOwner:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setGroupThreadOwner(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getGroupThreadOwner:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.getGroupThreadOwner(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setGroupThreadConferenceUri:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.net.Uri _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setGroupThreadConferenceUri(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getGroupThreadConferenceUri:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.net.Uri _result = this.getGroupThreadConferenceUri(_arg0, _arg1);
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
        case TRANSACTION_addParticipantToGroupThread:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.addParticipantToGroupThread(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeParticipantFromGroupThread:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.removeParticipantFromGroupThread(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_createRcsParticipant:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _result = this.createRcsParticipant(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getRcsParticipantCanonicalAddress:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _result = this.getRcsParticipantCanonicalAddress(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getRcsParticipantAlias:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _result = this.getRcsParticipantAlias(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_setRcsParticipantAlias:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setRcsParticipantAlias(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getRcsParticipantContactId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _result = this.getRcsParticipantContactId(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_setRcsParticipantContactId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setRcsParticipantContactId(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setMessageSubId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.setMessageSubId(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getMessageSubId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _result = this.getMessageSubId(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setMessageStatus:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.setMessageStatus(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getMessageStatus:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _result = this.getMessageStatus(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setMessageOriginationTimestamp:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          long _arg2;
          _arg2 = data.readLong();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.setMessageOriginationTimestamp(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getMessageOriginationTimestamp:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.lang.String _arg2;
          _arg2 = data.readString();
          long _result = this.getMessageOriginationTimestamp(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_setGlobalMessageIdForMessage:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.setGlobalMessageIdForMessage(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getGlobalMessageIdForMessage:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String _result = this.getGlobalMessageIdForMessage(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_setMessageArrivalTimestamp:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          long _arg2;
          _arg2 = data.readLong();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.setMessageArrivalTimestamp(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getMessageArrivalTimestamp:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.lang.String _arg2;
          _arg2 = data.readString();
          long _result = this.getMessageArrivalTimestamp(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_setMessageSeenTimestamp:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          long _arg2;
          _arg2 = data.readLong();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.setMessageSeenTimestamp(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getMessageSeenTimestamp:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.lang.String _arg2;
          _arg2 = data.readString();
          long _result = this.getMessageSeenTimestamp(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_setTextForMessage:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.setTextForMessage(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getTextForMessage:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String _result = this.getTextForMessage(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_setLatitudeForMessage:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          double _arg2;
          _arg2 = data.readDouble();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.setLatitudeForMessage(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getLatitudeForMessage:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.lang.String _arg2;
          _arg2 = data.readString();
          double _result = this.getLatitudeForMessage(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeDouble(_result);
          return true;
        }
        case TRANSACTION_setLongitudeForMessage:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          double _arg2;
          _arg2 = data.readDouble();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.setLongitudeForMessage(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getLongitudeForMessage:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.lang.String _arg2;
          _arg2 = data.readString();
          double _result = this.getLongitudeForMessage(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeDouble(_result);
          return true;
        }
        case TRANSACTION_getFileTransfersAttachedToMessage:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.lang.String _arg2;
          _arg2 = data.readString();
          int[] _result = this.getFileTransfersAttachedToMessage(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_getSenderParticipant:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.getSenderParticipant(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getMessageRecipients:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int[] _result = this.getMessageRecipients(_arg0, _arg1);
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_getOutgoingDeliveryDeliveredTimestamp:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          long _result = this.getOutgoingDeliveryDeliveredTimestamp(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_setOutgoingDeliveryDeliveredTimestamp:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          long _arg2;
          _arg2 = data.readLong();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.setOutgoingDeliveryDeliveredTimestamp(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getOutgoingDeliverySeenTimestamp:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          long _result = this.getOutgoingDeliverySeenTimestamp(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_setOutgoingDeliverySeenTimestamp:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          long _arg2;
          _arg2 = data.readLong();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.setOutgoingDeliverySeenTimestamp(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getOutgoingDeliveryStatus:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _result = this.getOutgoingDeliveryStatus(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setOutgoingDeliveryStatus:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.setOutgoingDeliveryStatus(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_storeFileTransfer:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          android.telephony.ims.RcsFileTransferCreationParams _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telephony.ims.RcsFileTransferCreationParams.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          java.lang.String _arg3;
          _arg3 = data.readString();
          int _result = this.storeFileTransfer(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_deleteFileTransfer:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.deleteFileTransfer(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setFileTransferSessionId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setFileTransferSessionId(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getFileTransferSessionId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _result = this.getFileTransferSessionId(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_setFileTransferContentUri:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.net.Uri _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setFileTransferContentUri(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getFileTransferContentUri:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.net.Uri _result = this.getFileTransferContentUri(_arg0, _arg1);
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
        case TRANSACTION_setFileTransferContentType:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setFileTransferContentType(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getFileTransferContentType:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _result = this.getFileTransferContentType(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_setFileTransferFileSize:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _arg1;
          _arg1 = data.readLong();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setFileTransferFileSize(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getFileTransferFileSize:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          long _result = this.getFileTransferFileSize(_arg0, _arg1);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_setFileTransferTransferOffset:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _arg1;
          _arg1 = data.readLong();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setFileTransferTransferOffset(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getFileTransferTransferOffset:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          long _result = this.getFileTransferTransferOffset(_arg0, _arg1);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_setFileTransferStatus:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setFileTransferStatus(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getFileTransferStatus:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.getFileTransferStatus(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setFileTransferWidth:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setFileTransferWidth(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getFileTransferWidth:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.getFileTransferWidth(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setFileTransferHeight:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setFileTransferHeight(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getFileTransferHeight:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.getFileTransferHeight(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setFileTransferLength:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _arg1;
          _arg1 = data.readLong();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setFileTransferLength(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getFileTransferLength:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          long _result = this.getFileTransferLength(_arg0, _arg1);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_setFileTransferPreviewUri:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.net.Uri _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setFileTransferPreviewUri(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getFileTransferPreviewUri:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.net.Uri _result = this.getFileTransferPreviewUri(_arg0, _arg1);
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
        case TRANSACTION_setFileTransferPreviewType:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setFileTransferPreviewType(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getFileTransferPreviewType:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _result = this.getFileTransferPreviewType(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_createGroupThreadNameChangedEvent:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          java.lang.String _arg4;
          _arg4 = data.readString();
          int _result = this.createGroupThreadNameChangedEvent(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_createGroupThreadIconChangedEvent:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          android.net.Uri _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          java.lang.String _arg4;
          _arg4 = data.readString();
          int _result = this.createGroupThreadIconChangedEvent(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_createGroupThreadParticipantJoinedEvent:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          java.lang.String _arg4;
          _arg4 = data.readString();
          int _result = this.createGroupThreadParticipantJoinedEvent(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_createGroupThreadParticipantLeftEvent:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          java.lang.String _arg4;
          _arg4 = data.readString();
          int _result = this.createGroupThreadParticipantLeftEvent(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_createParticipantAliasChangedEvent:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String _arg3;
          _arg3 = data.readString();
          int _result = this.createParticipantAliasChangedEvent(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.ims.aidl.IRcs
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
      /////////////////////////
      // RcsMessageStore APIs
      /////////////////////////

      @Override public android.telephony.ims.RcsThreadQueryResultParcelable getRcsThreads(android.telephony.ims.RcsThreadQueryParams queryParams, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.ims.RcsThreadQueryResultParcelable _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((queryParams!=null)) {
            _data.writeInt(1);
            queryParams.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRcsThreads, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRcsThreads(queryParams, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telephony.ims.RcsThreadQueryResultParcelable.CREATOR.createFromParcel(_reply);
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
      @Override public android.telephony.ims.RcsThreadQueryResultParcelable getRcsThreadsWithToken(android.telephony.ims.RcsQueryContinuationToken continuationToken, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.ims.RcsThreadQueryResultParcelable _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((continuationToken!=null)) {
            _data.writeInt(1);
            continuationToken.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRcsThreadsWithToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRcsThreadsWithToken(continuationToken, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telephony.ims.RcsThreadQueryResultParcelable.CREATOR.createFromParcel(_reply);
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
      @Override public android.telephony.ims.RcsParticipantQueryResultParcelable getParticipants(android.telephony.ims.RcsParticipantQueryParams queryParams, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.ims.RcsParticipantQueryResultParcelable _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((queryParams!=null)) {
            _data.writeInt(1);
            queryParams.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getParticipants, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getParticipants(queryParams, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telephony.ims.RcsParticipantQueryResultParcelable.CREATOR.createFromParcel(_reply);
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
      @Override public android.telephony.ims.RcsParticipantQueryResultParcelable getParticipantsWithToken(android.telephony.ims.RcsQueryContinuationToken continuationToken, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.ims.RcsParticipantQueryResultParcelable _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((continuationToken!=null)) {
            _data.writeInt(1);
            continuationToken.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getParticipantsWithToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getParticipantsWithToken(continuationToken, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telephony.ims.RcsParticipantQueryResultParcelable.CREATOR.createFromParcel(_reply);
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
      @Override public android.telephony.ims.RcsMessageQueryResultParcelable getMessages(android.telephony.ims.RcsMessageQueryParams queryParams, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.ims.RcsMessageQueryResultParcelable _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((queryParams!=null)) {
            _data.writeInt(1);
            queryParams.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMessages, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMessages(queryParams, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telephony.ims.RcsMessageQueryResultParcelable.CREATOR.createFromParcel(_reply);
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
      @Override public android.telephony.ims.RcsMessageQueryResultParcelable getMessagesWithToken(android.telephony.ims.RcsQueryContinuationToken continuationToken, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.ims.RcsMessageQueryResultParcelable _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((continuationToken!=null)) {
            _data.writeInt(1);
            continuationToken.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMessagesWithToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMessagesWithToken(continuationToken, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telephony.ims.RcsMessageQueryResultParcelable.CREATOR.createFromParcel(_reply);
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
      @Override public android.telephony.ims.RcsEventQueryResultDescriptor getEvents(android.telephony.ims.RcsEventQueryParams queryParams, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.ims.RcsEventQueryResultDescriptor _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((queryParams!=null)) {
            _data.writeInt(1);
            queryParams.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getEvents, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getEvents(queryParams, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telephony.ims.RcsEventQueryResultDescriptor.CREATOR.createFromParcel(_reply);
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
      @Override public android.telephony.ims.RcsEventQueryResultDescriptor getEventsWithToken(android.telephony.ims.RcsQueryContinuationToken continuationToken, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.ims.RcsEventQueryResultDescriptor _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((continuationToken!=null)) {
            _data.writeInt(1);
            continuationToken.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getEventsWithToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getEventsWithToken(continuationToken, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telephony.ims.RcsEventQueryResultDescriptor.CREATOR.createFromParcel(_reply);
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
      // returns true if the thread was successfully deleted

      @Override public boolean deleteThread(int threadId, int threadType, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(threadId);
          _data.writeInt(threadType);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_deleteThread, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().deleteThread(threadId, threadType, callingPackage);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // Creates an Rcs1To1Thread and returns its row ID

      @Override public int createRcs1To1Thread(int participantId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(participantId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createRcs1To1Thread, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createRcs1To1Thread(participantId, callingPackage);
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
      // Creates an RcsGroupThread and returns its row ID

      @Override public int createGroupThread(int[] participantIds, java.lang.String groupName, android.net.Uri groupIcon, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeIntArray(participantIds);
          _data.writeString(groupName);
          if ((groupIcon!=null)) {
            _data.writeInt(1);
            groupIcon.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createGroupThread, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createGroupThread(participantIds, groupName, groupIcon, callingPackage);
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
      /////////////////////////
      // RcsThread APIs
      /////////////////////////
      // Creates a new RcsIncomingMessage on the given thread and returns its row ID

      @Override public int addIncomingMessage(int rcsThreadId, android.telephony.ims.RcsIncomingMessageCreationParams rcsIncomingMessageCreationParams, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(rcsThreadId);
          if ((rcsIncomingMessageCreationParams!=null)) {
            _data.writeInt(1);
            rcsIncomingMessageCreationParams.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addIncomingMessage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addIncomingMessage(rcsThreadId, rcsIncomingMessageCreationParams, callingPackage);
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
      // Creates a new RcsOutgoingMessage on the given thread and returns its row ID

      @Override public int addOutgoingMessage(int rcsThreadId, android.telephony.ims.RcsOutgoingMessageCreationParams rcsOutgoingMessageCreationParams, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(rcsThreadId);
          if ((rcsOutgoingMessageCreationParams!=null)) {
            _data.writeInt(1);
            rcsOutgoingMessageCreationParams.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addOutgoingMessage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addOutgoingMessage(rcsThreadId, rcsOutgoingMessageCreationParams, callingPackage);
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
      // TODO: modify RcsProvider URI's to allow deleting a message without specifying its thread

      @Override public void deleteMessage(int rcsMessageId, boolean isIncoming, int rcsThreadId, boolean isGroup, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(rcsMessageId);
          _data.writeInt(((isIncoming)?(1):(0)));
          _data.writeInt(rcsThreadId);
          _data.writeInt(((isGroup)?(1):(0)));
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_deleteMessage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deleteMessage(rcsMessageId, isIncoming, rcsThreadId, isGroup, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.telephony.ims.RcsMessageSnippet getMessageSnippet(int rcsThreadId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.ims.RcsMessageSnippet _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(rcsThreadId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMessageSnippet, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMessageSnippet(rcsThreadId, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telephony.ims.RcsMessageSnippet.CREATOR.createFromParcel(_reply);
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
      /////////////////////////
      // Rcs1To1Thread APIs
      /////////////////////////

      @Override public void set1To1ThreadFallbackThreadId(int rcsThreadId, long fallbackId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(rcsThreadId);
          _data.writeLong(fallbackId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_set1To1ThreadFallbackThreadId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().set1To1ThreadFallbackThreadId(rcsThreadId, fallbackId, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public long get1To1ThreadFallbackThreadId(int rcsThreadId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(rcsThreadId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_get1To1ThreadFallbackThreadId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().get1To1ThreadFallbackThreadId(rcsThreadId, callingPackage);
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int get1To1ThreadOtherParticipantId(int rcsThreadId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(rcsThreadId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_get1To1ThreadOtherParticipantId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().get1To1ThreadOtherParticipantId(rcsThreadId, callingPackage);
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
      /////////////////////////
      // RcsGroupThread APIs
      /////////////////////////

      @Override public void setGroupThreadName(int rcsThreadId, java.lang.String groupName, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(rcsThreadId);
          _data.writeString(groupName);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setGroupThreadName, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setGroupThreadName(rcsThreadId, groupName, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String getGroupThreadName(int rcsThreadId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(rcsThreadId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getGroupThreadName, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getGroupThreadName(rcsThreadId, callingPackage);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setGroupThreadIcon(int rcsThreadId, android.net.Uri groupIcon, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(rcsThreadId);
          if ((groupIcon!=null)) {
            _data.writeInt(1);
            groupIcon.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setGroupThreadIcon, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setGroupThreadIcon(rcsThreadId, groupIcon, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.net.Uri getGroupThreadIcon(int rcsThreadId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.Uri _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(rcsThreadId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getGroupThreadIcon, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getGroupThreadIcon(rcsThreadId, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.Uri.CREATOR.createFromParcel(_reply);
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
      @Override public void setGroupThreadOwner(int rcsThreadId, int participantId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(rcsThreadId);
          _data.writeInt(participantId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setGroupThreadOwner, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setGroupThreadOwner(rcsThreadId, participantId, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getGroupThreadOwner(int rcsThreadId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(rcsThreadId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getGroupThreadOwner, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getGroupThreadOwner(rcsThreadId, callingPackage);
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
      @Override public void setGroupThreadConferenceUri(int rcsThreadId, android.net.Uri conferenceUri, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(rcsThreadId);
          if ((conferenceUri!=null)) {
            _data.writeInt(1);
            conferenceUri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setGroupThreadConferenceUri, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setGroupThreadConferenceUri(rcsThreadId, conferenceUri, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.net.Uri getGroupThreadConferenceUri(int rcsThreadId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.Uri _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(rcsThreadId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getGroupThreadConferenceUri, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getGroupThreadConferenceUri(rcsThreadId, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.Uri.CREATOR.createFromParcel(_reply);
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
      @Override public void addParticipantToGroupThread(int rcsThreadId, int participantId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(rcsThreadId);
          _data.writeInt(participantId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addParticipantToGroupThread, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addParticipantToGroupThread(rcsThreadId, participantId, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeParticipantFromGroupThread(int rcsThreadId, int participantId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(rcsThreadId);
          _data.writeInt(participantId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeParticipantFromGroupThread, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeParticipantFromGroupThread(rcsThreadId, participantId, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /////////////////////////
      // RcsParticipant APIs
      /////////////////////////
      // Creates a new RcsParticipant and returns its rowId

      @Override public int createRcsParticipant(java.lang.String canonicalAddress, java.lang.String alias, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(canonicalAddress);
          _data.writeString(alias);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createRcsParticipant, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createRcsParticipant(canonicalAddress, alias, callingPackage);
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
      @Override public java.lang.String getRcsParticipantCanonicalAddress(int participantId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(participantId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRcsParticipantCanonicalAddress, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRcsParticipantCanonicalAddress(participantId, callingPackage);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String getRcsParticipantAlias(int participantId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(participantId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRcsParticipantAlias, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRcsParticipantAlias(participantId, callingPackage);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setRcsParticipantAlias(int id, java.lang.String alias, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(id);
          _data.writeString(alias);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRcsParticipantAlias, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setRcsParticipantAlias(id, alias, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String getRcsParticipantContactId(int participantId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(participantId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRcsParticipantContactId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRcsParticipantContactId(participantId, callingPackage);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setRcsParticipantContactId(int participantId, java.lang.String contactId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(participantId);
          _data.writeString(contactId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRcsParticipantContactId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setRcsParticipantContactId(participantId, contactId, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /////////////////////////
      // RcsMessage APIs
      /////////////////////////

      @Override public void setMessageSubId(int messageId, boolean isIncoming, int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(((isIncoming)?(1):(0)));
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setMessageSubId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setMessageSubId(messageId, isIncoming, subId, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getMessageSubId(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(((isIncoming)?(1):(0)));
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMessageSubId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMessageSubId(messageId, isIncoming, callingPackage);
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
      @Override public void setMessageStatus(int messageId, boolean isIncoming, int status, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(((isIncoming)?(1):(0)));
          _data.writeInt(status);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setMessageStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setMessageStatus(messageId, isIncoming, status, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getMessageStatus(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(((isIncoming)?(1):(0)));
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMessageStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMessageStatus(messageId, isIncoming, callingPackage);
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
      @Override public void setMessageOriginationTimestamp(int messageId, boolean isIncoming, long originationTimestamp, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(((isIncoming)?(1):(0)));
          _data.writeLong(originationTimestamp);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setMessageOriginationTimestamp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setMessageOriginationTimestamp(messageId, isIncoming, originationTimestamp, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public long getMessageOriginationTimestamp(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(((isIncoming)?(1):(0)));
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMessageOriginationTimestamp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMessageOriginationTimestamp(messageId, isIncoming, callingPackage);
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setGlobalMessageIdForMessage(int messageId, boolean isIncoming, java.lang.String globalId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(((isIncoming)?(1):(0)));
          _data.writeString(globalId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setGlobalMessageIdForMessage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setGlobalMessageIdForMessage(messageId, isIncoming, globalId, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String getGlobalMessageIdForMessage(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(((isIncoming)?(1):(0)));
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getGlobalMessageIdForMessage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getGlobalMessageIdForMessage(messageId, isIncoming, callingPackage);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setMessageArrivalTimestamp(int messageId, boolean isIncoming, long arrivalTimestamp, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(((isIncoming)?(1):(0)));
          _data.writeLong(arrivalTimestamp);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setMessageArrivalTimestamp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setMessageArrivalTimestamp(messageId, isIncoming, arrivalTimestamp, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public long getMessageArrivalTimestamp(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(((isIncoming)?(1):(0)));
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMessageArrivalTimestamp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMessageArrivalTimestamp(messageId, isIncoming, callingPackage);
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setMessageSeenTimestamp(int messageId, boolean isIncoming, long seenTimestamp, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(((isIncoming)?(1):(0)));
          _data.writeLong(seenTimestamp);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setMessageSeenTimestamp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setMessageSeenTimestamp(messageId, isIncoming, seenTimestamp, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public long getMessageSeenTimestamp(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(((isIncoming)?(1):(0)));
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMessageSeenTimestamp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMessageSeenTimestamp(messageId, isIncoming, callingPackage);
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setTextForMessage(int messageId, boolean isIncoming, java.lang.String text, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(((isIncoming)?(1):(0)));
          _data.writeString(text);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTextForMessage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setTextForMessage(messageId, isIncoming, text, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String getTextForMessage(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(((isIncoming)?(1):(0)));
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTextForMessage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTextForMessage(messageId, isIncoming, callingPackage);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setLatitudeForMessage(int messageId, boolean isIncoming, double latitude, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(((isIncoming)?(1):(0)));
          _data.writeDouble(latitude);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setLatitudeForMessage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setLatitudeForMessage(messageId, isIncoming, latitude, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public double getLatitudeForMessage(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        double _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(((isIncoming)?(1):(0)));
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLatitudeForMessage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLatitudeForMessage(messageId, isIncoming, callingPackage);
          }
          _reply.readException();
          _result = _reply.readDouble();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setLongitudeForMessage(int messageId, boolean isIncoming, double longitude, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(((isIncoming)?(1):(0)));
          _data.writeDouble(longitude);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setLongitudeForMessage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setLongitudeForMessage(messageId, isIncoming, longitude, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public double getLongitudeForMessage(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        double _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(((isIncoming)?(1):(0)));
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLongitudeForMessage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLongitudeForMessage(messageId, isIncoming, callingPackage);
          }
          _reply.readException();
          _result = _reply.readDouble();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // Returns the ID's of the file transfers attached to the given message

      @Override public int[] getFileTransfersAttachedToMessage(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(((isIncoming)?(1):(0)));
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFileTransfersAttachedToMessage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFileTransfersAttachedToMessage(messageId, isIncoming, callingPackage);
          }
          _reply.readException();
          _result = _reply.createIntArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int getSenderParticipant(int messageId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSenderParticipant, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSenderParticipant(messageId, callingPackage);
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
      /////////////////////////
      // RcsOutgoingMessageDelivery APIs
      /////////////////////////
      // Returns the participant ID's that this message is intended to be delivered to

      @Override public int[] getMessageRecipients(int messageId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMessageRecipients, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMessageRecipients(messageId, callingPackage);
          }
          _reply.readException();
          _result = _reply.createIntArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public long getOutgoingDeliveryDeliveredTimestamp(int messageId, int participantId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(participantId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getOutgoingDeliveryDeliveredTimestamp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getOutgoingDeliveryDeliveredTimestamp(messageId, participantId, callingPackage);
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setOutgoingDeliveryDeliveredTimestamp(int messageId, int participantId, long deliveredTimestamp, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(participantId);
          _data.writeLong(deliveredTimestamp);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setOutgoingDeliveryDeliveredTimestamp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setOutgoingDeliveryDeliveredTimestamp(messageId, participantId, deliveredTimestamp, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public long getOutgoingDeliverySeenTimestamp(int messageId, int participantId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(participantId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getOutgoingDeliverySeenTimestamp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getOutgoingDeliverySeenTimestamp(messageId, participantId, callingPackage);
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setOutgoingDeliverySeenTimestamp(int messageId, int participantId, long seenTimestamp, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(participantId);
          _data.writeLong(seenTimestamp);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setOutgoingDeliverySeenTimestamp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setOutgoingDeliverySeenTimestamp(messageId, participantId, seenTimestamp, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getOutgoingDeliveryStatus(int messageId, int participantId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(participantId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getOutgoingDeliveryStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getOutgoingDeliveryStatus(messageId, participantId, callingPackage);
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
      @Override public void setOutgoingDeliveryStatus(int messageId, int participantId, int status, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(participantId);
          _data.writeInt(status);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setOutgoingDeliveryStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setOutgoingDeliveryStatus(messageId, participantId, status, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /////////////////////////
      // RcsFileTransferPart APIs
      /////////////////////////
      // Performs the initial write to storage and returns the row ID.

      @Override public int storeFileTransfer(int messageId, boolean isIncoming, android.telephony.ims.RcsFileTransferCreationParams fileTransferCreationParams, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(messageId);
          _data.writeInt(((isIncoming)?(1):(0)));
          if ((fileTransferCreationParams!=null)) {
            _data.writeInt(1);
            fileTransferCreationParams.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_storeFileTransfer, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().storeFileTransfer(messageId, isIncoming, fileTransferCreationParams, callingPackage);
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
      @Override public void deleteFileTransfer(int partId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(partId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_deleteFileTransfer, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deleteFileTransfer(partId, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setFileTransferSessionId(int partId, java.lang.String sessionId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(partId);
          _data.writeString(sessionId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFileTransferSessionId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFileTransferSessionId(partId, sessionId, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String getFileTransferSessionId(int partId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(partId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFileTransferSessionId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFileTransferSessionId(partId, callingPackage);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setFileTransferContentUri(int partId, android.net.Uri contentUri, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(partId);
          if ((contentUri!=null)) {
            _data.writeInt(1);
            contentUri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFileTransferContentUri, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFileTransferContentUri(partId, contentUri, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.net.Uri getFileTransferContentUri(int partId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.Uri _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(partId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFileTransferContentUri, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFileTransferContentUri(partId, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.Uri.CREATOR.createFromParcel(_reply);
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
      @Override public void setFileTransferContentType(int partId, java.lang.String contentType, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(partId);
          _data.writeString(contentType);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFileTransferContentType, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFileTransferContentType(partId, contentType, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String getFileTransferContentType(int partId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(partId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFileTransferContentType, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFileTransferContentType(partId, callingPackage);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setFileTransferFileSize(int partId, long fileSize, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(partId);
          _data.writeLong(fileSize);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFileTransferFileSize, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFileTransferFileSize(partId, fileSize, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public long getFileTransferFileSize(int partId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(partId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFileTransferFileSize, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFileTransferFileSize(partId, callingPackage);
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setFileTransferTransferOffset(int partId, long transferOffset, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(partId);
          _data.writeLong(transferOffset);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFileTransferTransferOffset, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFileTransferTransferOffset(partId, transferOffset, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public long getFileTransferTransferOffset(int partId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(partId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFileTransferTransferOffset, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFileTransferTransferOffset(partId, callingPackage);
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setFileTransferStatus(int partId, int transferStatus, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(partId);
          _data.writeInt(transferStatus);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFileTransferStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFileTransferStatus(partId, transferStatus, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getFileTransferStatus(int partId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(partId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFileTransferStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFileTransferStatus(partId, callingPackage);
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
      @Override public void setFileTransferWidth(int partId, int width, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(partId);
          _data.writeInt(width);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFileTransferWidth, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFileTransferWidth(partId, width, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getFileTransferWidth(int partId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(partId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFileTransferWidth, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFileTransferWidth(partId, callingPackage);
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
      @Override public void setFileTransferHeight(int partId, int height, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(partId);
          _data.writeInt(height);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFileTransferHeight, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFileTransferHeight(partId, height, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getFileTransferHeight(int partId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(partId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFileTransferHeight, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFileTransferHeight(partId, callingPackage);
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
      @Override public void setFileTransferLength(int partId, long length, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(partId);
          _data.writeLong(length);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFileTransferLength, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFileTransferLength(partId, length, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public long getFileTransferLength(int partId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(partId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFileTransferLength, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFileTransferLength(partId, callingPackage);
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setFileTransferPreviewUri(int partId, android.net.Uri uri, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(partId);
          if ((uri!=null)) {
            _data.writeInt(1);
            uri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFileTransferPreviewUri, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFileTransferPreviewUri(partId, uri, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.net.Uri getFileTransferPreviewUri(int partId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.Uri _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(partId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFileTransferPreviewUri, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFileTransferPreviewUri(partId, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.Uri.CREATOR.createFromParcel(_reply);
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
      @Override public void setFileTransferPreviewType(int partId, java.lang.String type, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(partId);
          _data.writeString(type);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setFileTransferPreviewType, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setFileTransferPreviewType(partId, type, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String getFileTransferPreviewType(int partId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(partId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFileTransferPreviewType, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFileTransferPreviewType(partId, callingPackage);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /////////////////////////
      // RcsEvent APIs
      /////////////////////////

      @Override public int createGroupThreadNameChangedEvent(long timestamp, int threadId, int originationParticipantId, java.lang.String newName, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(timestamp);
          _data.writeInt(threadId);
          _data.writeInt(originationParticipantId);
          _data.writeString(newName);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createGroupThreadNameChangedEvent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createGroupThreadNameChangedEvent(timestamp, threadId, originationParticipantId, newName, callingPackage);
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
      @Override public int createGroupThreadIconChangedEvent(long timestamp, int threadId, int originationParticipantId, android.net.Uri newIcon, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(timestamp);
          _data.writeInt(threadId);
          _data.writeInt(originationParticipantId);
          if ((newIcon!=null)) {
            _data.writeInt(1);
            newIcon.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createGroupThreadIconChangedEvent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createGroupThreadIconChangedEvent(timestamp, threadId, originationParticipantId, newIcon, callingPackage);
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
      @Override public int createGroupThreadParticipantJoinedEvent(long timestamp, int threadId, int originationParticipantId, int participantId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(timestamp);
          _data.writeInt(threadId);
          _data.writeInt(originationParticipantId);
          _data.writeInt(participantId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createGroupThreadParticipantJoinedEvent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createGroupThreadParticipantJoinedEvent(timestamp, threadId, originationParticipantId, participantId, callingPackage);
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
      @Override public int createGroupThreadParticipantLeftEvent(long timestamp, int threadId, int originationParticipantId, int participantId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(timestamp);
          _data.writeInt(threadId);
          _data.writeInt(originationParticipantId);
          _data.writeInt(participantId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createGroupThreadParticipantLeftEvent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createGroupThreadParticipantLeftEvent(timestamp, threadId, originationParticipantId, participantId, callingPackage);
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
      @Override public int createParticipantAliasChangedEvent(long timestamp, int participantId, java.lang.String newAlias, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(timestamp);
          _data.writeInt(participantId);
          _data.writeString(newAlias);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createParticipantAliasChangedEvent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createParticipantAliasChangedEvent(timestamp, participantId, newAlias, callingPackage);
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
      public static android.telephony.ims.aidl.IRcs sDefaultImpl;
    }
    static final int TRANSACTION_getRcsThreads = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getRcsThreadsWithToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getParticipants = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getParticipantsWithToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getMessages = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getMessagesWithToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_getEvents = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_getEventsWithToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_deleteThread = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_createRcs1To1Thread = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_createGroupThread = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_addIncomingMessage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_addOutgoingMessage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_deleteMessage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_getMessageSnippet = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_set1To1ThreadFallbackThreadId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_get1To1ThreadFallbackThreadId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_get1To1ThreadOtherParticipantId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_setGroupThreadName = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_getGroupThreadName = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_setGroupThreadIcon = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_getGroupThreadIcon = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_setGroupThreadOwner = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_getGroupThreadOwner = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_setGroupThreadConferenceUri = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_getGroupThreadConferenceUri = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_addParticipantToGroupThread = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_removeParticipantFromGroupThread = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_createRcsParticipant = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_getRcsParticipantCanonicalAddress = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_getRcsParticipantAlias = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_setRcsParticipantAlias = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_getRcsParticipantContactId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_setRcsParticipantContactId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_setMessageSubId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_getMessageSubId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_setMessageStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    static final int TRANSACTION_getMessageStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 37);
    static final int TRANSACTION_setMessageOriginationTimestamp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 38);
    static final int TRANSACTION_getMessageOriginationTimestamp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 39);
    static final int TRANSACTION_setGlobalMessageIdForMessage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 40);
    static final int TRANSACTION_getGlobalMessageIdForMessage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 41);
    static final int TRANSACTION_setMessageArrivalTimestamp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 42);
    static final int TRANSACTION_getMessageArrivalTimestamp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 43);
    static final int TRANSACTION_setMessageSeenTimestamp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 44);
    static final int TRANSACTION_getMessageSeenTimestamp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 45);
    static final int TRANSACTION_setTextForMessage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 46);
    static final int TRANSACTION_getTextForMessage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 47);
    static final int TRANSACTION_setLatitudeForMessage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 48);
    static final int TRANSACTION_getLatitudeForMessage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 49);
    static final int TRANSACTION_setLongitudeForMessage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 50);
    static final int TRANSACTION_getLongitudeForMessage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 51);
    static final int TRANSACTION_getFileTransfersAttachedToMessage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 52);
    static final int TRANSACTION_getSenderParticipant = (android.os.IBinder.FIRST_CALL_TRANSACTION + 53);
    static final int TRANSACTION_getMessageRecipients = (android.os.IBinder.FIRST_CALL_TRANSACTION + 54);
    static final int TRANSACTION_getOutgoingDeliveryDeliveredTimestamp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 55);
    static final int TRANSACTION_setOutgoingDeliveryDeliveredTimestamp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 56);
    static final int TRANSACTION_getOutgoingDeliverySeenTimestamp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 57);
    static final int TRANSACTION_setOutgoingDeliverySeenTimestamp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 58);
    static final int TRANSACTION_getOutgoingDeliveryStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 59);
    static final int TRANSACTION_setOutgoingDeliveryStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 60);
    static final int TRANSACTION_storeFileTransfer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 61);
    static final int TRANSACTION_deleteFileTransfer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 62);
    static final int TRANSACTION_setFileTransferSessionId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 63);
    static final int TRANSACTION_getFileTransferSessionId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 64);
    static final int TRANSACTION_setFileTransferContentUri = (android.os.IBinder.FIRST_CALL_TRANSACTION + 65);
    static final int TRANSACTION_getFileTransferContentUri = (android.os.IBinder.FIRST_CALL_TRANSACTION + 66);
    static final int TRANSACTION_setFileTransferContentType = (android.os.IBinder.FIRST_CALL_TRANSACTION + 67);
    static final int TRANSACTION_getFileTransferContentType = (android.os.IBinder.FIRST_CALL_TRANSACTION + 68);
    static final int TRANSACTION_setFileTransferFileSize = (android.os.IBinder.FIRST_CALL_TRANSACTION + 69);
    static final int TRANSACTION_getFileTransferFileSize = (android.os.IBinder.FIRST_CALL_TRANSACTION + 70);
    static final int TRANSACTION_setFileTransferTransferOffset = (android.os.IBinder.FIRST_CALL_TRANSACTION + 71);
    static final int TRANSACTION_getFileTransferTransferOffset = (android.os.IBinder.FIRST_CALL_TRANSACTION + 72);
    static final int TRANSACTION_setFileTransferStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 73);
    static final int TRANSACTION_getFileTransferStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 74);
    static final int TRANSACTION_setFileTransferWidth = (android.os.IBinder.FIRST_CALL_TRANSACTION + 75);
    static final int TRANSACTION_getFileTransferWidth = (android.os.IBinder.FIRST_CALL_TRANSACTION + 76);
    static final int TRANSACTION_setFileTransferHeight = (android.os.IBinder.FIRST_CALL_TRANSACTION + 77);
    static final int TRANSACTION_getFileTransferHeight = (android.os.IBinder.FIRST_CALL_TRANSACTION + 78);
    static final int TRANSACTION_setFileTransferLength = (android.os.IBinder.FIRST_CALL_TRANSACTION + 79);
    static final int TRANSACTION_getFileTransferLength = (android.os.IBinder.FIRST_CALL_TRANSACTION + 80);
    static final int TRANSACTION_setFileTransferPreviewUri = (android.os.IBinder.FIRST_CALL_TRANSACTION + 81);
    static final int TRANSACTION_getFileTransferPreviewUri = (android.os.IBinder.FIRST_CALL_TRANSACTION + 82);
    static final int TRANSACTION_setFileTransferPreviewType = (android.os.IBinder.FIRST_CALL_TRANSACTION + 83);
    static final int TRANSACTION_getFileTransferPreviewType = (android.os.IBinder.FIRST_CALL_TRANSACTION + 84);
    static final int TRANSACTION_createGroupThreadNameChangedEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 85);
    static final int TRANSACTION_createGroupThreadIconChangedEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 86);
    static final int TRANSACTION_createGroupThreadParticipantJoinedEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 87);
    static final int TRANSACTION_createGroupThreadParticipantLeftEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 88);
    static final int TRANSACTION_createParticipantAliasChangedEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 89);
    public static boolean setDefaultImpl(android.telephony.ims.aidl.IRcs impl) {
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
    public static android.telephony.ims.aidl.IRcs getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /////////////////////////
  // RcsMessageStore APIs
  /////////////////////////

  public android.telephony.ims.RcsThreadQueryResultParcelable getRcsThreads(android.telephony.ims.RcsThreadQueryParams queryParams, java.lang.String callingPackage) throws android.os.RemoteException;
  public android.telephony.ims.RcsThreadQueryResultParcelable getRcsThreadsWithToken(android.telephony.ims.RcsQueryContinuationToken continuationToken, java.lang.String callingPackage) throws android.os.RemoteException;
  public android.telephony.ims.RcsParticipantQueryResultParcelable getParticipants(android.telephony.ims.RcsParticipantQueryParams queryParams, java.lang.String callingPackage) throws android.os.RemoteException;
  public android.telephony.ims.RcsParticipantQueryResultParcelable getParticipantsWithToken(android.telephony.ims.RcsQueryContinuationToken continuationToken, java.lang.String callingPackage) throws android.os.RemoteException;
  public android.telephony.ims.RcsMessageQueryResultParcelable getMessages(android.telephony.ims.RcsMessageQueryParams queryParams, java.lang.String callingPackage) throws android.os.RemoteException;
  public android.telephony.ims.RcsMessageQueryResultParcelable getMessagesWithToken(android.telephony.ims.RcsQueryContinuationToken continuationToken, java.lang.String callingPackage) throws android.os.RemoteException;
  public android.telephony.ims.RcsEventQueryResultDescriptor getEvents(android.telephony.ims.RcsEventQueryParams queryParams, java.lang.String callingPackage) throws android.os.RemoteException;
  public android.telephony.ims.RcsEventQueryResultDescriptor getEventsWithToken(android.telephony.ims.RcsQueryContinuationToken continuationToken, java.lang.String callingPackage) throws android.os.RemoteException;
  // returns true if the thread was successfully deleted

  public boolean deleteThread(int threadId, int threadType, java.lang.String callingPackage) throws android.os.RemoteException;
  // Creates an Rcs1To1Thread and returns its row ID

  public int createRcs1To1Thread(int participantId, java.lang.String callingPackage) throws android.os.RemoteException;
  // Creates an RcsGroupThread and returns its row ID

  public int createGroupThread(int[] participantIds, java.lang.String groupName, android.net.Uri groupIcon, java.lang.String callingPackage) throws android.os.RemoteException;
  /////////////////////////
  // RcsThread APIs
  /////////////////////////
  // Creates a new RcsIncomingMessage on the given thread and returns its row ID

  public int addIncomingMessage(int rcsThreadId, android.telephony.ims.RcsIncomingMessageCreationParams rcsIncomingMessageCreationParams, java.lang.String callingPackage) throws android.os.RemoteException;
  // Creates a new RcsOutgoingMessage on the given thread and returns its row ID

  public int addOutgoingMessage(int rcsThreadId, android.telephony.ims.RcsOutgoingMessageCreationParams rcsOutgoingMessageCreationParams, java.lang.String callingPackage) throws android.os.RemoteException;
  // TODO: modify RcsProvider URI's to allow deleting a message without specifying its thread

  public void deleteMessage(int rcsMessageId, boolean isIncoming, int rcsThreadId, boolean isGroup, java.lang.String callingPackage) throws android.os.RemoteException;
  public android.telephony.ims.RcsMessageSnippet getMessageSnippet(int rcsThreadId, java.lang.String callingPackage) throws android.os.RemoteException;
  /////////////////////////
  // Rcs1To1Thread APIs
  /////////////////////////

  public void set1To1ThreadFallbackThreadId(int rcsThreadId, long fallbackId, java.lang.String callingPackage) throws android.os.RemoteException;
  public long get1To1ThreadFallbackThreadId(int rcsThreadId, java.lang.String callingPackage) throws android.os.RemoteException;
  public int get1To1ThreadOtherParticipantId(int rcsThreadId, java.lang.String callingPackage) throws android.os.RemoteException;
  /////////////////////////
  // RcsGroupThread APIs
  /////////////////////////

  public void setGroupThreadName(int rcsThreadId, java.lang.String groupName, java.lang.String callingPackage) throws android.os.RemoteException;
  public java.lang.String getGroupThreadName(int rcsThreadId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setGroupThreadIcon(int rcsThreadId, android.net.Uri groupIcon, java.lang.String callingPackage) throws android.os.RemoteException;
  public android.net.Uri getGroupThreadIcon(int rcsThreadId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setGroupThreadOwner(int rcsThreadId, int participantId, java.lang.String callingPackage) throws android.os.RemoteException;
  public int getGroupThreadOwner(int rcsThreadId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setGroupThreadConferenceUri(int rcsThreadId, android.net.Uri conferenceUri, java.lang.String callingPackage) throws android.os.RemoteException;
  public android.net.Uri getGroupThreadConferenceUri(int rcsThreadId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void addParticipantToGroupThread(int rcsThreadId, int participantId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void removeParticipantFromGroupThread(int rcsThreadId, int participantId, java.lang.String callingPackage) throws android.os.RemoteException;
  /////////////////////////
  // RcsParticipant APIs
  /////////////////////////
  // Creates a new RcsParticipant and returns its rowId

  public int createRcsParticipant(java.lang.String canonicalAddress, java.lang.String alias, java.lang.String callingPackage) throws android.os.RemoteException;
  public java.lang.String getRcsParticipantCanonicalAddress(int participantId, java.lang.String callingPackage) throws android.os.RemoteException;
  public java.lang.String getRcsParticipantAlias(int participantId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setRcsParticipantAlias(int id, java.lang.String alias, java.lang.String callingPackage) throws android.os.RemoteException;
  public java.lang.String getRcsParticipantContactId(int participantId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setRcsParticipantContactId(int participantId, java.lang.String contactId, java.lang.String callingPackage) throws android.os.RemoteException;
  /////////////////////////
  // RcsMessage APIs
  /////////////////////////

  public void setMessageSubId(int messageId, boolean isIncoming, int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  public int getMessageSubId(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setMessageStatus(int messageId, boolean isIncoming, int status, java.lang.String callingPackage) throws android.os.RemoteException;
  public int getMessageStatus(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setMessageOriginationTimestamp(int messageId, boolean isIncoming, long originationTimestamp, java.lang.String callingPackage) throws android.os.RemoteException;
  public long getMessageOriginationTimestamp(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setGlobalMessageIdForMessage(int messageId, boolean isIncoming, java.lang.String globalId, java.lang.String callingPackage) throws android.os.RemoteException;
  public java.lang.String getGlobalMessageIdForMessage(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setMessageArrivalTimestamp(int messageId, boolean isIncoming, long arrivalTimestamp, java.lang.String callingPackage) throws android.os.RemoteException;
  public long getMessageArrivalTimestamp(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setMessageSeenTimestamp(int messageId, boolean isIncoming, long seenTimestamp, java.lang.String callingPackage) throws android.os.RemoteException;
  public long getMessageSeenTimestamp(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setTextForMessage(int messageId, boolean isIncoming, java.lang.String text, java.lang.String callingPackage) throws android.os.RemoteException;
  public java.lang.String getTextForMessage(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setLatitudeForMessage(int messageId, boolean isIncoming, double latitude, java.lang.String callingPackage) throws android.os.RemoteException;
  public double getLatitudeForMessage(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setLongitudeForMessage(int messageId, boolean isIncoming, double longitude, java.lang.String callingPackage) throws android.os.RemoteException;
  public double getLongitudeForMessage(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException;
  // Returns the ID's of the file transfers attached to the given message

  public int[] getFileTransfersAttachedToMessage(int messageId, boolean isIncoming, java.lang.String callingPackage) throws android.os.RemoteException;
  public int getSenderParticipant(int messageId, java.lang.String callingPackage) throws android.os.RemoteException;
  /////////////////////////
  // RcsOutgoingMessageDelivery APIs
  /////////////////////////
  // Returns the participant ID's that this message is intended to be delivered to

  public int[] getMessageRecipients(int messageId, java.lang.String callingPackage) throws android.os.RemoteException;
  public long getOutgoingDeliveryDeliveredTimestamp(int messageId, int participantId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setOutgoingDeliveryDeliveredTimestamp(int messageId, int participantId, long deliveredTimestamp, java.lang.String callingPackage) throws android.os.RemoteException;
  public long getOutgoingDeliverySeenTimestamp(int messageId, int participantId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setOutgoingDeliverySeenTimestamp(int messageId, int participantId, long seenTimestamp, java.lang.String callingPackage) throws android.os.RemoteException;
  public int getOutgoingDeliveryStatus(int messageId, int participantId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setOutgoingDeliveryStatus(int messageId, int participantId, int status, java.lang.String callingPackage) throws android.os.RemoteException;
  /////////////////////////
  // RcsFileTransferPart APIs
  /////////////////////////
  // Performs the initial write to storage and returns the row ID.

  public int storeFileTransfer(int messageId, boolean isIncoming, android.telephony.ims.RcsFileTransferCreationParams fileTransferCreationParams, java.lang.String callingPackage) throws android.os.RemoteException;
  public void deleteFileTransfer(int partId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setFileTransferSessionId(int partId, java.lang.String sessionId, java.lang.String callingPackage) throws android.os.RemoteException;
  public java.lang.String getFileTransferSessionId(int partId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setFileTransferContentUri(int partId, android.net.Uri contentUri, java.lang.String callingPackage) throws android.os.RemoteException;
  public android.net.Uri getFileTransferContentUri(int partId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setFileTransferContentType(int partId, java.lang.String contentType, java.lang.String callingPackage) throws android.os.RemoteException;
  public java.lang.String getFileTransferContentType(int partId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setFileTransferFileSize(int partId, long fileSize, java.lang.String callingPackage) throws android.os.RemoteException;
  public long getFileTransferFileSize(int partId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setFileTransferTransferOffset(int partId, long transferOffset, java.lang.String callingPackage) throws android.os.RemoteException;
  public long getFileTransferTransferOffset(int partId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setFileTransferStatus(int partId, int transferStatus, java.lang.String callingPackage) throws android.os.RemoteException;
  public int getFileTransferStatus(int partId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setFileTransferWidth(int partId, int width, java.lang.String callingPackage) throws android.os.RemoteException;
  public int getFileTransferWidth(int partId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setFileTransferHeight(int partId, int height, java.lang.String callingPackage) throws android.os.RemoteException;
  public int getFileTransferHeight(int partId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setFileTransferLength(int partId, long length, java.lang.String callingPackage) throws android.os.RemoteException;
  public long getFileTransferLength(int partId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setFileTransferPreviewUri(int partId, android.net.Uri uri, java.lang.String callingPackage) throws android.os.RemoteException;
  public android.net.Uri getFileTransferPreviewUri(int partId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void setFileTransferPreviewType(int partId, java.lang.String type, java.lang.String callingPackage) throws android.os.RemoteException;
  public java.lang.String getFileTransferPreviewType(int partId, java.lang.String callingPackage) throws android.os.RemoteException;
  /////////////////////////
  // RcsEvent APIs
  /////////////////////////

  public int createGroupThreadNameChangedEvent(long timestamp, int threadId, int originationParticipantId, java.lang.String newName, java.lang.String callingPackage) throws android.os.RemoteException;
  public int createGroupThreadIconChangedEvent(long timestamp, int threadId, int originationParticipantId, android.net.Uri newIcon, java.lang.String callingPackage) throws android.os.RemoteException;
  public int createGroupThreadParticipantJoinedEvent(long timestamp, int threadId, int originationParticipantId, int participantId, java.lang.String callingPackage) throws android.os.RemoteException;
  public int createGroupThreadParticipantLeftEvent(long timestamp, int threadId, int originationParticipantId, int participantId, java.lang.String callingPackage) throws android.os.RemoteException;
  public int createParticipantAliasChangedEvent(long timestamp, int participantId, java.lang.String newAlias, java.lang.String callingPackage) throws android.os.RemoteException;
}
