/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.telephony;
/**
 * Interface used to interact with the phone.  Mostly this is used by the
 * TelephonyManager class.  A few places are still using this directly.
 * Please clean them up if possible and use TelephonyManager instead.
 *
 * {@hide}
 */
public interface ITelephony extends android.os.IInterface
{
  /** Default implementation for ITelephony. */
  public static class Default implements com.android.internal.telephony.ITelephony
  {
    /**
         * Dial a number. This doesn't place the call. It displays
         * the Dialer screen.
         * @param number the number to be dialed. If null, this
         * would display the Dialer screen with no number pre-filled.
         */
    @Override public void dial(java.lang.String number) throws android.os.RemoteException
    {
    }
    /**
         * Place a call to the specified number.
         * @param callingPackage The package making the call.
         * @param number the number to be called.
         */
    @Override public void call(java.lang.String callingPackage, java.lang.String number) throws android.os.RemoteException
    {
    }
    /**
         * Check to see if the radio is on or not.
         * @param callingPackage the name of the package making the call.
         * @return returns true if the radio is on.
         */
    @Override public boolean isRadioOn(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Check to see if the radio is on or not on particular subId.
         * @param subId user preferred subId.
         * @param callingPackage the name of the package making the call.
         * @return returns true if the radio is on.
         */
    @Override public boolean isRadioOnForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Supply a pin to unlock the SIM.  Blocks until a result is determined.
         * @param pin The pin to check.
         * @return whether the operation was a success.
         */
    @Override public boolean supplyPin(java.lang.String pin) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Supply a pin to unlock the SIM for particular subId.
         * Blocks until a result is determined.
         * @param pin The pin to check.
         * @param subId user preferred subId.
         * @return whether the operation was a success.
         */
    @Override public boolean supplyPinForSubscriber(int subId, java.lang.String pin) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Supply puk to unlock the SIM and set SIM pin to new pin.
         *  Blocks until a result is determined.
         * @param puk The puk to check.
         *        pin The new pin to be set in SIM
         * @return whether the operation was a success.
         */
    @Override public boolean supplyPuk(java.lang.String puk, java.lang.String pin) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Supply puk to unlock the SIM and set SIM pin to new pin.
         *  Blocks until a result is determined.
         * @param puk The puk to check.
         *        pin The new pin to be set in SIM
         * @param subId user preferred subId.
         * @return whether the operation was a success.
         */
    @Override public boolean supplyPukForSubscriber(int subId, java.lang.String puk, java.lang.String pin) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Supply a pin to unlock the SIM.  Blocks until a result is determined.
         * Returns a specific success/error code.
         * @param pin The pin to check.
         * @return retValue[0] = Phone.PIN_RESULT_SUCCESS on success. Otherwise error code
         *         retValue[1] = number of attempts remaining if known otherwise -1
         */
    @Override public int[] supplyPinReportResult(java.lang.String pin) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Supply a pin to unlock the SIM.  Blocks until a result is determined.
         * Returns a specific success/error code.
         * @param pin The pin to check.
         * @return retValue[0] = Phone.PIN_RESULT_SUCCESS on success. Otherwise error code
         *         retValue[1] = number of attempts remaining if known otherwise -1
         */
    @Override public int[] supplyPinReportResultForSubscriber(int subId, java.lang.String pin) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Supply puk to unlock the SIM and set SIM pin to new pin.
         * Blocks until a result is determined.
         * Returns a specific success/error code
         * @param puk The puk to check
         *        pin The pin to check.
         * @return retValue[0] = Phone.PIN_RESULT_SUCCESS on success. Otherwise error code
         *         retValue[1] = number of attempts remaining if known otherwise -1
         */
    @Override public int[] supplyPukReportResult(java.lang.String puk, java.lang.String pin) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Supply puk to unlock the SIM and set SIM pin to new pin.
         * Blocks until a result is determined.
         * Returns a specific success/error code
         * @param puk The puk to check
         *        pin The pin to check.
         * @return retValue[0] = Phone.PIN_RESULT_SUCCESS on success. Otherwise error code
         *         retValue[1] = number of attempts remaining if known otherwise -1
         */
    @Override public int[] supplyPukReportResultForSubscriber(int subId, java.lang.String puk, java.lang.String pin) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Handles PIN MMI commands (PIN/PIN2/PUK/PUK2), which are initiated
         * without SEND (so <code>dial</code> is not appropriate).
         *
         * @param dialString the MMI command to be executed.
         * @return true if MMI command is executed.
         */
    @Override public boolean handlePinMmi(java.lang.String dialString) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Handles USSD commands.
         *
         * @param subId The subscription to use.
         * @param ussdRequest the USSD command to be executed.
         * @param wrappedCallback receives a callback result.
         */
    @Override public void handleUssdRequest(int subId, java.lang.String ussdRequest, android.os.ResultReceiver wrappedCallback) throws android.os.RemoteException
    {
    }
    /**
         * Handles PIN MMI commands (PIN/PIN2/PUK/PUK2), which are initiated
         * without SEND (so <code>dial</code> is not appropriate) for
         * a particular subId.
         * @param dialString the MMI command to be executed.
         * @param subId user preferred subId.
         * @return true if MMI command is executed.
         */
    @Override public boolean handlePinMmiForSubscriber(int subId, java.lang.String dialString) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Toggles the radio on or off.
         */
    @Override public void toggleRadioOnOff() throws android.os.RemoteException
    {
    }
    /**
         * Toggles the radio on or off on particular subId.
         * @param subId user preferred subId.
         */
    @Override public void toggleRadioOnOffForSubscriber(int subId) throws android.os.RemoteException
    {
    }
    /**
         * Set the radio to on or off
         */
    @Override public boolean setRadio(boolean turnOn) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Set the radio to on or off on particular subId.
         * @param subId user preferred subId.
         */
    @Override public boolean setRadioForSubscriber(int subId, boolean turnOn) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Set the radio to on or off unconditionally
         */
    @Override public boolean setRadioPower(boolean turnOn) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Request to update location information in service state
         */
    @Override public void updateServiceLocation() throws android.os.RemoteException
    {
    }
    /**
         * Request to update location information for a subscrition in service state
         * @param subId user preferred subId.
         */
    @Override public void updateServiceLocationForSubscriber(int subId) throws android.os.RemoteException
    {
    }
    /**
         * Enable location update notifications.
         */
    @Override public void enableLocationUpdates() throws android.os.RemoteException
    {
    }
    /**
         * Enable location update notifications.
         * @param subId user preferred subId.
         */
    @Override public void enableLocationUpdatesForSubscriber(int subId) throws android.os.RemoteException
    {
    }
    /**
         * Disable location update notifications.
         */
    @Override public void disableLocationUpdates() throws android.os.RemoteException
    {
    }
    /**
         * Disable location update notifications.
         * @param subId user preferred subId.
         */
    @Override public void disableLocationUpdatesForSubscriber(int subId) throws android.os.RemoteException
    {
    }
    /**
         * Allow mobile data connections.
         */
    @Override public boolean enableDataConnectivity() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Disallow mobile data connections.
         */
    @Override public boolean disableDataConnectivity() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Report whether data connectivity is possible.
         */
    @Override public boolean isDataConnectivityPossible(int subId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.os.Bundle getCellLocation(java.lang.String callingPkg) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Returns the ISO country code equivalent of the current registered
         * operator's MCC (Mobile Country Code).
         * @see android.telephony.TelephonyManager#getNetworkCountryIso
         */
    @Override public java.lang.String getNetworkCountryIsoForPhone(int phoneId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Returns the neighboring cell information of the device.
         */
    @Override public java.util.List<android.telephony.NeighboringCellInfo> getNeighboringCellInfo(java.lang.String callingPkg) throws android.os.RemoteException
    {
      return null;
    }
    @Override public int getCallState() throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Returns the call state for a slot.
         */
    @Override public int getCallStateForSlot(int slotIndex) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Replaced by getDataActivityForSubId.
         */
    @Override public int getDataActivity() throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Returns a constant indicating the type of activity on a data connection
         * (cellular).
         *
         * @see #DATA_ACTIVITY_NONE
         * @see #DATA_ACTIVITY_IN
         * @see #DATA_ACTIVITY_OUT
         * @see #DATA_ACTIVITY_INOUT
         * @see #DATA_ACTIVITY_DORMANT
         */
    @Override public int getDataActivityForSubId(int subId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Replaced by getDataStateForSubId.
         */
    @Override public int getDataState() throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Returns a constant indicating the current data connection state
         * (cellular).
         *
         * @see #DATA_DISCONNECTED
         * @see #DATA_CONNECTING
         * @see #DATA_CONNECTED
         * @see #DATA_SUSPENDED
         */
    @Override public int getDataStateForSubId(int subId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Returns the current active phone type as integer.
         * Returns TelephonyManager.PHONE_TYPE_CDMA if RILConstants.CDMA_PHONE
         * and TelephonyManager.PHONE_TYPE_GSM if RILConstants.GSM_PHONE
         */
    @Override public int getActivePhoneType() throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Returns the current active phone type as integer for particular slot.
         * Returns TelephonyManager.PHONE_TYPE_CDMA if RILConstants.CDMA_PHONE
         * and TelephonyManager.PHONE_TYPE_GSM if RILConstants.GSM_PHONE
         * @param slotIndex - slot to query.
         */
    @Override public int getActivePhoneTypeForSlot(int slotIndex) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Returns the CDMA ERI icon index to display
         * @param callingPackage package making the call.
         */
    @Override public int getCdmaEriIconIndex(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Returns the CDMA ERI icon index to display on particular subId.
         * @param subId user preferred subId.
         * @param callingPackage package making the call.
         */
    @Override public int getCdmaEriIconIndexForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Returns the CDMA ERI icon mode,
         * 0 - ON
         * 1 - FLASHING
         * @param callingPackage package making the call.
         */
    @Override public int getCdmaEriIconMode(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Returns the CDMA ERI icon mode on particular subId,
         * 0 - ON
         * 1 - FLASHING
         * @param subId user preferred subId.
         * @param callingPackage package making the call.
         */
    @Override public int getCdmaEriIconModeForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Returns the CDMA ERI text,
         * @param callingPackage package making the call.
         */
    @Override public java.lang.String getCdmaEriText(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Returns the CDMA ERI text for particular subId,
         * @param subId user preferred subId.
         * @param callingPackage package making the call.
         */
    @Override public java.lang.String getCdmaEriTextForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Returns true if OTA service provisioning needs to run.
         * Only relevant on some technologies, others will always
         * return false.
         */
    @Override public boolean needsOtaServiceProvisioning() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Sets the voicemail number for a particular subscriber.
         */
    @Override public boolean setVoiceMailNumber(int subId, java.lang.String alphaTag, java.lang.String number) throws android.os.RemoteException
    {
      return false;
    }
    /**
          * Sets the voice activation state for a particular subscriber.
          */
    @Override public void setVoiceActivationState(int subId, int activationState) throws android.os.RemoteException
    {
    }
    /**
          * Sets the data activation state for a particular subscriber.
          */
    @Override public void setDataActivationState(int subId, int activationState) throws android.os.RemoteException
    {
    }
    /**
          * Returns the voice activation state for a particular subscriber.
          * @param subId user preferred sub
          * @param callingPackage package queries voice activation state
          */
    @Override public int getVoiceActivationState(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /**
          * Returns the data activation state for a particular subscriber.
          * @param subId user preferred sub
          * @param callingPackage package queris data activation state
          */
    @Override public int getDataActivationState(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Returns the unread count of voicemails for a subId.
         * @param subId user preferred subId.
         * Returns the unread count of voicemails
         */
    @Override public int getVoiceMessageCountForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /**
          * Returns true if current state supports both voice and data
          * simultaneously. This can change based on location or network condition.
          */
    @Override public boolean isConcurrentVoiceAndDataAllowed(int subId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.os.Bundle getVisualVoicemailSettings(java.lang.String callingPackage, int subId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String getVisualVoicemailPackageName(java.lang.String callingPackage, int subId) throws android.os.RemoteException
    {
      return null;
    }
    // Not oneway, caller needs to make sure the vaule is set before receiving a SMS

    @Override public void enableVisualVoicemailSmsFilter(java.lang.String callingPackage, int subId, android.telephony.VisualVoicemailSmsFilterSettings settings) throws android.os.RemoteException
    {
    }
    @Override public void disableVisualVoicemailSmsFilter(java.lang.String callingPackage, int subId) throws android.os.RemoteException
    {
    }
    // Get settings set by the calling package

    @Override public android.telephony.VisualVoicemailSmsFilterSettings getVisualVoicemailSmsFilterSettings(java.lang.String callingPackage, int subId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         *  Get settings set by the current default dialer, Internal use only.
         *  Requires READ_PRIVILEGED_PHONE_STATE permission.
         */
    @Override public android.telephony.VisualVoicemailSmsFilterSettings getActiveVisualVoicemailSmsFilterSettings(int subId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Send a visual voicemail SMS. Internal use only.
         * Requires caller to be the default dialer and have SEND_SMS permission
         */
    @Override public void sendVisualVoicemailSmsForSubscriber(java.lang.String callingPackage, int subId, java.lang.String number, int port, java.lang.String text, android.app.PendingIntent sentIntent) throws android.os.RemoteException
    {
    }
    // Send the special dialer code. The IPC caller must be the current default dialer.

    @Override public void sendDialerSpecialCode(java.lang.String callingPackageName, java.lang.String inputCode) throws android.os.RemoteException
    {
    }
    /**
         * Returns the network type of a subId.
         * @param subId user preferred subId.
         * @param callingPackage package making the call.
         */
    @Override public int getNetworkTypeForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Returns the network type for data transmission
         * @param callingPackage package making the call.
         */
    @Override public int getDataNetworkType(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Returns the data network type of a subId
         * @param subId user preferred subId.
         * @param callingPackage package making the call.
         */
    @Override public int getDataNetworkTypeForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /**
          * Returns the voice network type of a subId
          * @param subId user preferred subId.
          * @param callingPackage package making the call.
          * Returns the network type
          */
    @Override public int getVoiceNetworkTypeForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Return true if an ICC card is present
         */
    @Override public boolean hasIccCard() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Return true if an ICC card is present for a subId.
         * @param slotIndex user preferred slotIndex.
         * Return true if an ICC card is present
         */
    @Override public boolean hasIccCardUsingSlotIndex(int slotIndex) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Return if the current radio is LTE on CDMA. This
         * is a tri-state return value as for a period of time
         * the mode may be unknown.
         *
         * @param callingPackage the name of the calling package
         * @return {@link Phone#LTE_ON_CDMA_UNKNOWN}, {@link Phone#LTE_ON_CDMA_FALSE}
         * or {@link PHone#LTE_ON_CDMA_TRUE}
         */
    @Override public int getLteOnCdmaMode(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Return if the current radio is LTE on CDMA. This
         * is a tri-state return value as for a period of time
         * the mode may be unknown.
         *
         * @param callingPackage the name of the calling package
         * @return {@link Phone#LTE_ON_CDMA_UNKNOWN}, {@link Phone#LTE_ON_CDMA_FALSE}
         * or {@link PHone#LTE_ON_CDMA_TRUE}
         */
    @Override public int getLteOnCdmaModeForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Returns all observed cell information of the device.
         */
    @Override public java.util.List<android.telephony.CellInfo> getAllCellInfo(java.lang.String callingPkg) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Request a cell information update for the specified subscription,
         * reported via the CellInfoCallback.
         */
    @Override public void requestCellInfoUpdate(int subId, android.telephony.ICellInfoCallback cb, java.lang.String callingPkg) throws android.os.RemoteException
    {
    }
    /**
         * Request a cell information update for the specified subscription,
         * reported via the CellInfoCallback.
         *
         * @param workSource the requestor to whom the power consumption for this should be attributed.
         */
    @Override public void requestCellInfoUpdateWithWorkSource(int subId, android.telephony.ICellInfoCallback cb, java.lang.String callingPkg, android.os.WorkSource ws) throws android.os.RemoteException
    {
    }
    /**
         * Sets minimum time in milli-seconds between onCellInfoChanged
         */
    @Override public void setCellInfoListRate(int rateInMillis) throws android.os.RemoteException
    {
    }
    /**
         * Opens a logical channel to the ICC card using the physical slot index.
         *
         * Input parameters equivalent to TS 27.007 AT+CCHO command.
         *
         * @param slotIndex The physical slot index of the target ICC card
         * @param callingPackage the name of the package making the call.
         * @param AID Application id. See ETSI 102.221 and 101.220.
         * @param p2 P2 parameter (described in ISO 7816-4).
         * @return an IccOpenLogicalChannelResponse object.
         */
    @Override public android.telephony.IccOpenLogicalChannelResponse iccOpenLogicalChannelBySlot(int slotIndex, java.lang.String callingPackage, java.lang.String AID, int p2) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Opens a logical channel to the ICC card.
         *
         * Input parameters equivalent to TS 27.007 AT+CCHO command.
         *
         * @param subId The subscription to use.
         * @param callingPackage the name of the package making the call.
         * @param AID Application id. See ETSI 102.221 and 101.220.
         * @param p2 P2 parameter (described in ISO 7816-4).
         * @return an IccOpenLogicalChannelResponse object.
         */
    @Override public android.telephony.IccOpenLogicalChannelResponse iccOpenLogicalChannel(int subId, java.lang.String callingPackage, java.lang.String AID, int p2) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Closes a previously opened logical channel to the ICC card using the physical slot index.
         *
         * Input parameters equivalent to TS 27.007 AT+CCHC command.
         *
         * @param slotIndex The physical slot index of the target ICC card
         * @param channel is the channel id to be closed as returned by a
         *            successful iccOpenLogicalChannel.
         * @return true if the channel was closed successfully.
         */
    @Override public boolean iccCloseLogicalChannelBySlot(int slotIndex, int channel) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Closes a previously opened logical channel to the ICC card.
         *
         * Input parameters equivalent to TS 27.007 AT+CCHC command.
         *
         * @param subId The subscription to use.
         * @param channel is the channel id to be closed as returned by a
         *            successful iccOpenLogicalChannel.
         * @return true if the channel was closed successfully.
         */
    @Override public boolean iccCloseLogicalChannel(int subId, int channel) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Transmit an APDU to the ICC card over a logical channel using the physical slot index.
         *
         * Input parameters equivalent to TS 27.007 AT+CGLA command.
         *
         * @param slotIndex The physical slot index of the target ICC card
         * @param channel is the channel id to be closed as returned by a
         *            successful iccOpenLogicalChannel.
         * @param cla Class of the APDU command.
         * @param instruction Instruction of the APDU command.
         * @param p1 P1 value of the APDU command.
         * @param p2 P2 value of the APDU command.
         * @param p3 P3 value of the APDU command. If p3 is negative a 4 byte APDU
         *            is sent to the SIM.
         * @param data Data to be sent with the APDU.
         * @return The APDU response from the ICC card with the status appended at
         *            the end.
         */
    @Override public java.lang.String iccTransmitApduLogicalChannelBySlot(int slotIndex, int channel, int cla, int instruction, int p1, int p2, int p3, java.lang.String data) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Transmit an APDU to the ICC card over a logical channel.
         *
         * Input parameters equivalent to TS 27.007 AT+CGLA command.
         *
         * @param subId The subscription to use.
         * @param channel is the channel id to be closed as returned by a
         *            successful iccOpenLogicalChannel.
         * @param cla Class of the APDU command.
         * @param instruction Instruction of the APDU command.
         * @param p1 P1 value of the APDU command.
         * @param p2 P2 value of the APDU command.
         * @param p3 P3 value of the APDU command. If p3 is negative a 4 byte APDU
         *            is sent to the SIM.
         * @param data Data to be sent with the APDU.
         * @return The APDU response from the ICC card with the status appended at
         *            the end.
         */
    @Override public java.lang.String iccTransmitApduLogicalChannel(int subId, int channel, int cla, int instruction, int p1, int p2, int p3, java.lang.String data) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Transmit an APDU to the ICC card over the basic channel using the physical slot index.
         *
         * Input parameters equivalent to TS 27.007 AT+CSIM command.
         *
         * @param slotIndex The physical slot index of the target ICC card
         * @param callingPackage the name of the package making the call.
         * @param cla Class of the APDU command.
         * @param instruction Instruction of the APDU command.
         * @param p1 P1 value of the APDU command.
         * @param p2 P2 value of the APDU command.
         * @param p3 P3 value of the APDU command. If p3 is negative a 4 byte APDU
         *            is sent to the SIM.
         * @param data Data to be sent with the APDU.
         * @return The APDU response from the ICC card with the status appended at
         *            the end.
         */
    @Override public java.lang.String iccTransmitApduBasicChannelBySlot(int slotIndex, java.lang.String callingPackage, int cla, int instruction, int p1, int p2, int p3, java.lang.String data) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Transmit an APDU to the ICC card over the basic channel.
         *
         * Input parameters equivalent to TS 27.007 AT+CSIM command.
         *
         * @param subId The subscription to use.
         * @param callingPackage the name of the package making the call.
         * @param cla Class of the APDU command.
         * @param instruction Instruction of the APDU command.
         * @param p1 P1 value of the APDU command.
         * @param p2 P2 value of the APDU command.
         * @param p3 P3 value of the APDU command. If p3 is negative a 4 byte APDU
         *            is sent to the SIM.
         * @param data Data to be sent with the APDU.
         * @return The APDU response from the ICC card with the status appended at
         *            the end.
         */
    @Override public java.lang.String iccTransmitApduBasicChannel(int subId, java.lang.String callingPackage, int cla, int instruction, int p1, int p2, int p3, java.lang.String data) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Returns the response APDU for a command APDU sent through SIM_IO.
         *
         * @param subId The subscription to use.
         * @param fileID
         * @param command
         * @param p1 P1 value of the APDU command.
         * @param p2 P2 value of the APDU command.
         * @param p3 P3 value of the APDU command.
         * @param filePath
         * @return The APDU response.
         */
    @Override public byte[] iccExchangeSimIO(int subId, int fileID, int command, int p1, int p2, int p3, java.lang.String filePath) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Send ENVELOPE to the SIM and returns the response.
         *
         * @param subId The subscription to use.
         * @param contents  String containing SAT/USAT response in hexadecimal
         *                  format starting with command tag. See TS 102 223 for
         *                  details.
         * @return The APDU response from the ICC card, with the last 4 bytes
         *         being the status word. If the command fails, returns an empty
         *         string.
         */
    @Override public java.lang.String sendEnvelopeWithStatus(int subId, java.lang.String content) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Read one of the NV items defined in {@link RadioNVItems} / {@code ril_nv_items.h}.
         * Used for device configuration by some CDMA operators.
         *
         * @param itemID the ID of the item to read.
         * @return the NV item as a String, or null on any failure.
         */
    @Override public java.lang.String nvReadItem(int itemID) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Write one of the NV items defined in {@link RadioNVItems} / {@code ril_nv_items.h}.
         * Used for device configuration by some CDMA operators.
         *
         * @param itemID the ID of the item to read.
         * @param itemValue the value to write, as a String.
         * @return true on success; false on any failure.
         */
    @Override public boolean nvWriteItem(int itemID, java.lang.String itemValue) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Update the CDMA Preferred Roaming List (PRL) in the radio NV storage.
         * Used for device configuration by some CDMA operators.
         *
         * @param preferredRoamingList byte array containing the new PRL.
         * @return true on success; false on any failure.
         */
    @Override public boolean nvWriteCdmaPrl(byte[] preferredRoamingList) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Rollback modem configurations to factory default except some config which are in whitelist.
         * Used for device configuration by some CDMA operators.
         *
         * <p>Requires Permission:
         * {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE} or that the calling
         * app has carrier privileges (see {@link #hasCarrierPrivileges}).
         *
         * @param slotIndex - device slot.
         * @return {@code true} on success; {@code false} on any failure.
         */
    @Override public boolean resetModemConfig(int slotIndex) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Generate a radio modem reset. Used for device configuration by some CDMA operators.
         * Different than {@link #setRadioPower(boolean)}, modem reboot will power down sim card.
         *
         * <p>Requires Permission:
         * {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE} or that the calling
         * app has carrier privileges (see {@link #hasCarrierPrivileges}).
         *
         * @param slotIndex - device slot.
         * @return {@code true} on success; {@code false} on any failure.
         */
    @Override public boolean rebootModem(int slotIndex) throws android.os.RemoteException
    {
      return false;
    }
    /*
         * Get the calculated preferred network type.
         * Used for device configuration by some CDMA operators.
         * @param callingPackage The package making the call.
         *
         * @return the calculated preferred network type, defined in RILConstants.java.
         */
    @Override public int getCalculatedPreferredNetworkType(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /*
         * Get the preferred network type.
         * Used for device configuration by some CDMA operators.
         *
         * @param subId the id of the subscription to query.
         * @return the preferred network type, defined in RILConstants.java.
         */
    @Override public int getPreferredNetworkType(int subId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Check whether DUN APN is required for tethering with subId.
         *
         * @param subId the id of the subscription to require tethering.
         * @return {@code true} if DUN APN is required for tethering.
         * @hide
         */
    @Override public boolean getTetherApnRequiredForSubscriber(int subId) throws android.os.RemoteException
    {
      return false;
    }
    /**
        * Enables framework IMS and triggers IMS Registration.
        */
    @Override public void enableIms(int slotId) throws android.os.RemoteException
    {
    }
    /**
        * Disables framework IMS and triggers IMS deregistration.
        */
    @Override public void disableIms(int slotId) throws android.os.RemoteException
    {
    }
    /**
         *  Get IImsMmTelFeature binder from ImsResolver that corresponds to the subId and MMTel feature
         *  as well as registering the MmTelFeature for callbacks using the IImsServiceFeatureCallback
         *  interface.
         */
    @Override public android.telephony.ims.aidl.IImsMmTelFeature getMmTelFeatureAndListen(int slotId, com.android.ims.internal.IImsServiceFeatureCallback callback) throws android.os.RemoteException
    {
      return null;
    }
    /**
         *  Get IImsRcsFeature binder from ImsResolver that corresponds to the subId and RCS feature
         *  as well as registering the RcsFeature for callbacks using the IImsServiceFeatureCallback
         *  interface.
         */
    @Override public android.telephony.ims.aidl.IImsRcsFeature getRcsFeatureAndListen(int slotId, com.android.ims.internal.IImsServiceFeatureCallback callback) throws android.os.RemoteException
    {
      return null;
    }
    /**
        * Returns the IImsRegistration associated with the slot and feature specified.
        */
    @Override public android.telephony.ims.aidl.IImsRegistration getImsRegistration(int slotId, int feature) throws android.os.RemoteException
    {
      return null;
    }
    /**
        * Returns the IImsConfig associated with the slot and feature specified.
        */
    @Override public android.telephony.ims.aidl.IImsConfig getImsConfig(int slotId, int feature) throws android.os.RemoteException
    {
      return null;
    }
    /**
        *  @return true if the ImsService to bind to for the slot id specified was set, false otherwise.
        */
    @Override public boolean setImsService(int slotId, boolean isCarrierImsService, java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    /**
        * @return the package name of the carrier/device ImsService associated with this slot.
        */
    @Override public java.lang.String getImsService(int slotId, boolean isCarrierImsService) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Set the network selection mode to automatic.
         *
         * @param subId the id of the subscription to update.
         */
    @Override public void setNetworkSelectionModeAutomatic(int subId) throws android.os.RemoteException
    {
    }
    /**
         * Perform a radio scan and return the list of avialble networks.
         *
         * @param subId the id of the subscription.
         * @return CellNetworkScanResult containing status of scan and networks.
         */
    @Override public com.android.internal.telephony.CellNetworkScanResult getCellNetworkScanResults(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Perform a radio network scan and return the id of this scan.
         *
         * @param subId the id of the subscription.
         * @param request Defines all the configs for network scan.
         * @param messenger Callback messages will be sent using this messenger.
         * @param binder the binder object instantiated in TelephonyManager.
         * @param callingPackage the calling package
         * @return An id for this scan.
         */
    @Override public int requestNetworkScan(int subId, android.telephony.NetworkScanRequest request, android.os.Messenger messenger, android.os.IBinder binder, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Stop an existing radio network scan.
         *
         * @param subId the id of the subscription.
         * @param scanId The id of the scan that is going to be stopped.
         */
    @Override public void stopNetworkScan(int subId, int scanId) throws android.os.RemoteException
    {
    }
    /**
         * Ask the radio to connect to the input network and change selection mode to manual.
         *
         * @param subId the id of the subscription.
         * @param operatorInfo the operator inforamtion, included the PLMN, long name and short name of
         * the operator to attach to.
         * @param persistSelection whether the selection will persist until reboot. If true, only allows
         * attaching to the selected PLMN until reboot; otherwise, attach to the chosen PLMN and resume
         * normal network selection next time.
         * @return {@code true} on success; {@code true} on any failure.
         */
    @Override public boolean setNetworkSelectionModeManual(int subId, com.android.internal.telephony.OperatorInfo operatorInfo, boolean persisSelection) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Set the preferred network type.
         * Used for device configuration by some CDMA operators.
         *
         * @param subId the id of the subscription to update.
         * @param networkType the preferred network type, defined in RILConstants.java.
         * @return true on success; false on any failure.
         */
    @Override public boolean setPreferredNetworkType(int subId, int networkType) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * User enable/disable Mobile Data.
         *
         * @param enable true to turn on, else false
         */
    @Override public void setUserDataEnabled(int subId, boolean enable) throws android.os.RemoteException
    {
    }
    /**
         * Get the user enabled state of Mobile Data.
         *
         * TODO: remove and use isUserDataEnabled.
         * This can't be removed now because some vendor codes
         * calls through ITelephony directly while they should
         * use TelephonyManager.
         *
         * @return true on enabled
         */
    @Override public boolean getDataEnabled(int subId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Get the user enabled state of Mobile Data.
         *
         * @return true on enabled
         */
    @Override public boolean isUserDataEnabled(int subId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Get the overall enabled state of Mobile Data.
         *
         * @return true on enabled
         */
    @Override public boolean isDataEnabled(int subId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Checks if manual network selection is allowed.
         *
         * @return {@code true} if manual network selection is allowed, otherwise return {@code false}.
         */
    @Override public boolean isManualNetworkSelectionAllowed(int subId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Get P-CSCF address from PCO after data connection is established or modified.
         * @param apnType the apnType, "ims" for IMS APN, "emergency" for EMERGENCY APN
         * @param callingPackage The package making the call.
         */
    @Override public java.lang.String[] getPcscfAddress(java.lang.String apnType, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Set IMS registration state
         */
    @Override public void setImsRegistrationState(boolean registered) throws android.os.RemoteException
    {
    }
    /**
         * Return MDN string for CDMA phone.
         * @param subId user preferred subId.
         */
    @Override public java.lang.String getCdmaMdn(int subId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Return MIN string for CDMA phone.
         * @param subId user preferred subId.
         */
    @Override public java.lang.String getCdmaMin(int subId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Request that the next incoming call from a number matching {@code range} be intercepted.
         * @param range The range of phone numbers the caller expects a phone call from.
         * @param timeoutMillis The amount of time to wait for such a call, or
         *                      {@link #MAX_NUMBER_VERIFICATION_TIMEOUT_MILLIS}, whichever is lesser.
         * @param callback the callback aidl
         * @param callingPackage the calling package name.
         */
    @Override public void requestNumberVerification(android.telephony.PhoneNumberRange range, long timeoutMillis, com.android.internal.telephony.INumberVerificationCallback callback, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    /**
         * Has the calling application been granted special privileges by the carrier.
         *
         * If any of the packages in the calling UID has carrier privileges, the
         * call will return true. This access is granted by the owner of the UICC
         * card and does not depend on the registered carrier.
         *
         * TODO: Add a link to documentation.
         *
         * @param subId The subscription to use.
         * @return carrier privilege status defined in TelephonyManager.
         */
    @Override public int getCarrierPrivilegeStatus(int subId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Similar to above, but check for the given uid.
         */
    @Override public int getCarrierPrivilegeStatusForUid(int subId, int uid) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Similar to above, but check for the package whose name is pkgName.
         */
    @Override public int checkCarrierPrivilegesForPackage(int subId, java.lang.String pkgName) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Similar to above, but check across all phones.
         */
    @Override public int checkCarrierPrivilegesForPackageAnyPhone(java.lang.String pkgName) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Returns list of the package names of the carrier apps that should handle the input intent
         * and have carrier privileges for the given phoneId.
         *
         * @param intent Intent that will be sent.
         * @param phoneId The phoneId on which the carrier app has carrier privileges.
         * @return list of carrier app package names that can handle the intent on phoneId.
         *         Returns null if there is an error and an empty list if there
         *         are no matching packages.
         */
    @Override public java.util.List<java.lang.String> getCarrierPackageNamesForIntentAndPhone(android.content.Intent intent, int phoneId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Set the line 1 phone number string and its alphatag for the current ICCID
         * for display purpose only, for example, displayed in Phone Status. It won't
         * change the actual MSISDN/MDN. To unset alphatag or number, pass in a null
         * value.
         *
         * @param subId the subscriber that the alphatag and dialing number belongs to.
         * @param alphaTag alpha-tagging of the dailing nubmer
         * @param number The dialing number
         * @return true if the operation was executed correctly.
         */
    @Override public boolean setLine1NumberForDisplayForSubscriber(int subId, java.lang.String alphaTag, java.lang.String number) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Returns the displayed dialing number string if it was set previously via
         * {@link #setLine1NumberForDisplay}. Otherwise returns null.
         *
         * @param subId whose dialing number for line 1 is returned.
         * @param callingPackage The package making the call.
         * @return the displayed dialing number if set, or null if not set.
         */
    @Override public java.lang.String getLine1NumberForDisplay(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Returns the displayed alphatag of the dialing number if it was set
         * previously via {@link #setLine1NumberForDisplay}. Otherwise returns null.
         *
         * @param subId whose alphatag associated with line 1 is returned.
         * @param callingPackage The package making the call.
         * @return the displayed alphatag of the dialing number if set, or null if
         *         not set.
         */
    @Override public java.lang.String getLine1AlphaTagForDisplay(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Return the set of subscriber IDs that should be considered "merged together" for data usage
         * purposes. This is commonly {@code null} to indicate no merging is required. Any returned
         * subscribers are sorted in a deterministic order.
         * <p>
         * The returned set of subscriber IDs will include the subscriber ID corresponding to this
         * TelephonyManager's subId.
         *
         * @hide
         */
    @Override public java.lang.String[] getMergedSubscriberIds(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @hide
         */
    @Override public java.lang.String[] getMergedSubscriberIdsFromGroup(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Override the operator branding for the current ICCID.
         *
         * Once set, whenever the SIM is present in the device, the service
         * provider name (SPN) and the operator name will both be replaced by the
         * brand value input. To unset the value, the same function should be
         * called with a null brand value.
         *
         * <p>Requires Permission:
         *   {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}
         *  or has to be carrier app - see #hasCarrierPrivileges.
         *
         * @param subId The subscription to use.
         * @param brand The brand name to display/set.
         * @return true if the operation was executed correctly.
         */
    @Override public boolean setOperatorBrandOverride(int subId, java.lang.String brand) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Override the roaming indicator for the current ICCID.
         *
         * Using this call, the carrier app (see #hasCarrierPrivileges) can override
         * the platform's notion of a network operator being considered roaming or not.
         * The change only affects the ICCID that was active when this call was made.
         *
         * If null is passed as any of the input, the corresponding value is deleted.
         *
         * <p>Requires that the caller have carrier privilege. See #hasCarrierPrivileges.
         *
         * @param subId for which the roaming overrides apply.
         * @param gsmRoamingList - List of MCCMNCs to be considered roaming for 3GPP RATs.
         * @param gsmNonRoamingList - List of MCCMNCs to be considered not roaming for 3GPP RATs.
         * @param cdmaRoamingList - List of SIDs to be considered roaming for 3GPP2 RATs.
         * @param cdmaNonRoamingList - List of SIDs to be considered not roaming for 3GPP2 RATs.
         * @return true if the operation was executed correctly.
         */
    @Override public boolean setRoamingOverride(int subId, java.util.List<java.lang.String> gsmRoamingList, java.util.List<java.lang.String> gsmNonRoamingList, java.util.List<java.lang.String> cdmaRoamingList, java.util.List<java.lang.String> cdmaNonRoamingList) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Returns the result and response from RIL for oem request
         *
         * @param oemReq the data is sent to ril.
         * @param oemResp the respose data from RIL.
         * @return negative value request was not handled or get error
         *         0 request was handled succesfully, but no response data
         *         positive value success, data length of response
         */
    @Override public int invokeOemRilRequestRaw(byte[] oemReq, byte[] oemResp) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Check if any mobile Radios need to be shutdown.
         *
         * @return true is any mobile radio needs to be shutdown
         */
    @Override public boolean needMobileRadioShutdown() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Shutdown Mobile Radios
         */
    @Override public void shutdownMobileRadios() throws android.os.RemoteException
    {
    }
    /**
         * Set phone radio type and access technology.
         *
         * @param rafs an RadioAccessFamily array to indicate all phone's
         *        new radio access family. The length of RadioAccessFamily
         *        must equ]]al to phone count.
         */
    @Override public void setRadioCapability(android.telephony.RadioAccessFamily[] rafs) throws android.os.RemoteException
    {
    }
    /**
         * Get phone radio type and access technology.
         *
         * @param phoneId which phone you want to get
         * @param callingPackage the name of the package making the call
         * @return phone radio type and access technology
         */
    @Override public int getRadioAccessFamily(int phoneId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Enables or disables video calling.
         *
         * @param enable Whether to enable video calling.
         */
    @Override public void enableVideoCalling(boolean enable) throws android.os.RemoteException
    {
    }
    /**
         * Whether video calling has been enabled by the user.
         *
         * @param callingPackage The package making the call.
         * @return {@code true} if the user has enabled video calling, {@code false} otherwise.
         */
    @Override public boolean isVideoCallingEnabled(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Whether the DTMF tone length can be changed.
         *
         * @param subId The subscription to use.
         * @param callingPackage The package making the call.
         * @return {@code true} if the DTMF tone length can be changed.
         */
    @Override public boolean canChangeDtmfToneLength(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Whether the device is a world phone.
         *
         * @param callingPackage The package making the call.
         * @return {@code true} if the devices is a world phone.
         */
    @Override public boolean isWorldPhone(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Whether the phone supports TTY mode.
         *
         * @return {@code true} if the device supports TTY mode.
         */
    @Override public boolean isTtyModeSupported() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isRttSupported(int subscriptionId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Whether the phone supports hearing aid compatibility.
         *
         * @return {@code true} if the device supports hearing aid compatibility.
         */
    @Override public boolean isHearingAidCompatibilitySupported() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Get IMS Registration Status on a particular subid.
         *
         * @param subId user preferred subId.
         *
         * @return {@code true} if the IMS status is registered.
         */
    @Override public boolean isImsRegistered(int subId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Returns the Status of Wi-Fi Calling for the subscription id specified.
         */
    @Override public boolean isWifiCallingAvailable(int subId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Returns the Status of VT (video telephony) for the subscription ID specified.
         */
    @Override public boolean isVideoTelephonyAvailable(int subId) throws android.os.RemoteException
    {
      return false;
    }
    /**
        * Returns the MMTEL IMS registration technology for the subsciption ID specified.
        */
    @Override public int getImsRegTechnologyForMmTel(int subId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
          * Returns the unique device ID of phone, for example, the IMEI for
          * GSM and the MEID for CDMA phones. Return null if device ID is not available.
          *
          * @param callingPackage The package making the call.
          * <p>Requires Permission:
          *   {@link android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE}
          */
    @Override public java.lang.String getDeviceId(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Returns the IMEI for the given slot.
         *
         * @param slotIndex - device slot.
         * @param callingPackage The package making the call.
         * <p>Requires Permission:
         *   {@link android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE}
         */
    @Override public java.lang.String getImeiForSlot(int slotIndex, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Returns the Type Allocation Code from the IMEI for the given slot.
         *
         * @param slotIndex - Which slot to retrieve the Type Allocation Code from.
         */
    @Override public java.lang.String getTypeAllocationCodeForSlot(int slotIndex) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Returns the MEID for the given slot.
         *
         * @param slotIndex - device slot.
         * @param callingPackage The package making the call.
         * <p>Requires Permission:
         *   {@link android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE}
         */
    @Override public java.lang.String getMeidForSlot(int slotIndex, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Returns the Manufacturer Code from the MEID for the given slot.
         *
         * @param slotIndex - Which slot to retrieve the Manufacturer Code from.
         */
    @Override public java.lang.String getManufacturerCodeForSlot(int slotIndex) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Returns the device software version.
         *
         * @param slotIndex - device slot.
         * @param callingPackage The package making the call.
         * <p>Requires Permission:
         *   {@link android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE}
         */
    @Override public java.lang.String getDeviceSoftwareVersionForSlot(int slotIndex, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Returns the subscription ID associated with the specified PhoneAccount.
         */
    @Override public int getSubIdForPhoneAccount(android.telecom.PhoneAccount phoneAccount) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Returns the PhoneAccountHandle associated with a subscription ID.
         */
    @Override public android.telecom.PhoneAccountHandle getPhoneAccountHandleForSubscriptionId(int subscriptionId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void factoryReset(int subId) throws android.os.RemoteException
    {
    }
    /**
         * Returns users's current locale based on the SIM.
         *
         * The returned string will be a well formed BCP-47 language tag, or {@code null}
         * if no locale could be derived.
         */
    @Override public java.lang.String getSimLocaleForSubscriber(int subId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Requests the modem activity info asynchronously.
         * The implementor is expected to reply with the
         * {@link android.telephony.ModemActivityInfo} object placed into the Bundle with the key
         * {@link android.telephony.TelephonyManager#MODEM_ACTIVITY_RESULT_KEY}.
         * The result code is ignored.
         */
    @Override public void requestModemActivityInfo(android.os.ResultReceiver result) throws android.os.RemoteException
    {
    }
    /**
         * Get the service state on specified subscription
         * @param subId Subscription id
         * @param callingPackage The package making the call
         * @return Service state on specified subscription.
         */
    @Override public android.telephony.ServiceState getServiceStateForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Returns the URI for the per-account voicemail ringtone set in Phone settings.
         *
         * @param accountHandle The handle for the {@link PhoneAccount} for which to retrieve the
         * voicemail ringtone.
         * @return The URI for the ringtone to play when receiving a voicemail from a specific
         * PhoneAccount.
         */
    @Override public android.net.Uri getVoicemailRingtoneUri(android.telecom.PhoneAccountHandle accountHandle) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Sets the per-account voicemail ringtone.
         *
         * <p>Requires that the calling app is the default dialer, or has carrier privileges, or
         * has permission {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}.
         *
         * @param phoneAccountHandle The handle for the {@link PhoneAccount} for which to set the
         * voicemail ringtone.
         * @param uri The URI for the ringtone to play when receiving a voicemail from a specific
         * PhoneAccount.
         */
    @Override public void setVoicemailRingtoneUri(java.lang.String callingPackage, android.telecom.PhoneAccountHandle phoneAccountHandle, android.net.Uri uri) throws android.os.RemoteException
    {
    }
    /**
         * Returns whether vibration is set for voicemail notification in Phone settings.
         *
         * @param accountHandle The handle for the {@link PhoneAccount} for which to retrieve the
         * voicemail vibration setting.
         * @return {@code true} if the vibration is set for this PhoneAccount, {@code false} otherwise.
         */
    @Override public boolean isVoicemailVibrationEnabled(android.telecom.PhoneAccountHandle accountHandle) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Sets the per-account preference whether vibration is enabled for voicemail notifications.
         *
         * <p>Requires that the calling app is the default dialer, or has carrier privileges, or
         * has permission {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}.
         *
         * @param phoneAccountHandle The handle for the {@link PhoneAccount} for which to set the
         * voicemail vibration setting.
         * @param enabled Whether to enable or disable vibration for voicemail notifications from a
         * specific PhoneAccount.
         */
    @Override public void setVoicemailVibrationEnabled(java.lang.String callingPackage, android.telecom.PhoneAccountHandle phoneAccountHandle, boolean enabled) throws android.os.RemoteException
    {
    }
    /**
         * Returns a list of packages that have carrier privileges for the specific phone.
         */
    @Override public java.util.List<java.lang.String> getPackagesWithCarrierPrivileges(int phoneId) throws android.os.RemoteException
    {
      return null;
    }
    /**
          * Returns a list of packages that have carrier privileges.
          */
    @Override public java.util.List<java.lang.String> getPackagesWithCarrierPrivilegesForAllPhones() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Return the application ID for the app type.
         *
         * @param subId the subscription ID that this request applies to.
         * @param appType the uicc app type,
         * @return Application ID for specificied app type or null if no uicc or error.
         */
    @Override public java.lang.String getAidForAppType(int subId, int appType) throws android.os.RemoteException
    {
      return null;
    }
    /**
        * Return the Electronic Serial Number.
        *
        * Requires that the calling app has READ_PRIVILEGED_PHONE_STATE permission
        *
        * @param subId the subscription ID that this request applies to.
        * @return ESN or null if error.
        * @hide
        */
    @Override public java.lang.String getEsn(int subId) throws android.os.RemoteException
    {
      return null;
    }
    /**
        * Return the Preferred Roaming List Version
        *
        * Requires that the calling app has READ_PRIVILEGED_PHONE_STATE permission
        * @param subId the subscription ID that this request applies to.
        * @return PRLVersion or null if error.
        * @hide
        */
    @Override public java.lang.String getCdmaPrlVersion(int subId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Get snapshot of Telephony histograms
         * @return List of Telephony histograms
         * Requires Permission:
         *   {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}
         * Or the calling app has carrier privileges.
         */
    @Override public java.util.List<android.telephony.TelephonyHistogram> getTelephonyHistograms() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Set the allowed carrier list and the excluded carrier list, indicating the priority between
         * the two lists.
         *
         * <p>Requires system privileges. In the future we may add this to carrier APIs.
         *
         * @return {@link #SET_CARRIER_RESTRICTION_SUCCESS} in case of success.
         * {@link #SET_CARRIER_RESTRICTION_NOT_SUPPORTED} if the modem does not support the
         * configuration. {@link #SET_CARRIER_RESTRICTION_ERROR} in all other error cases.
         */
    @Override public int setAllowedCarriers(android.telephony.CarrierRestrictionRules carrierRestrictionRules) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Get the allowed carrier list and the excluded carrier list indicating the priority between
         * the two lists.
         *
         * <p>Requires system privileges. In the future we may add this to carrier APIs.
         *
         * @return {@link CarrierRestrictionRules}; empty lists mean all carriers are allowed. It
         * returns null in case of error.
         */
    @Override public android.telephony.CarrierRestrictionRules getAllowedCarriers() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Returns carrier id of the given subscription.
         * <p>To recognize carrier as a first class identity, assign each carrier with a canonical
         * integer a.k.a carrier id.
         *
         * @param subId The subscription id
         * @return Carrier id of given subscription id. return {@link #UNKNOWN_CARRIER_ID} if
         * subscription is unavailable or carrier cannot be identified.
         * @throws IllegalStateException if telephony service is unavailable.
         * @hide
         */
    @Override public int getSubscriptionCarrierId(int subId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Returns carrier name of the given subscription.
         * <p>Carrier name is a user-facing name of carrier id {@link #getSimCarrierId(int)},
         * usually the brand name of the subsidiary (e.g. T-Mobile). Each carrier could configure
         * multiple {@link #getSimOperatorName() SPN} but should have a single carrier name.
         * Carrier name is not canonical identity, use {@link #getSimCarrierId(int)} instead.
         * <p>Returned carrier name is unlocalized.
         *
         * @return Carrier name of given subscription id. return {@code null} if subscription is
         * unavailable or carrier cannot be identified.
         * @throws IllegalStateException if telephony service is unavailable.
         * @hide
         */
    @Override public java.lang.String getSubscriptionCarrierName(int subId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Returns fine-grained carrier id of the current subscription.
         *
         * <p>The specific carrier id can be used to further differentiate a carrier by different
         * networks, by prepaid v.s.postpaid or even by 4G v.s.3G plan. Each carrier has a unique
         * carrier id {@link #getSimCarrierId()} but can have multiple precise carrier id. e.g,
         * {@link #getSimCarrierId()} will always return Tracfone (id 2022) for a Tracfone SIM, while
         * {@link #getSimPreciseCarrierId()} can return Tracfone AT&T or Tracfone T-Mobile based on the
         * current underlying network.
         *
         * <p>For carriers without any fine-grained carrier ids, return {@link #getSimCarrierId()}
         *
         * @return Returns fine-grained carrier id of the current subscription.
         * Return {@link #UNKNOWN_CARRIER_ID} if the subscription is unavailable or the carrier cannot
         * be identified.
         * @hide
         */
    @Override public int getSubscriptionSpecificCarrierId(int subId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Similar like {@link #getSimCarrierIdName()}, returns user-facing name of the
         * specific carrier id {@link #getSimSpecificCarrierId()}
         *
         * <p>The returned name is unlocalized.
         *
         * @return user-facing name of the subscription specific carrier id. Return {@code null} if the
         * subscription is unavailable or the carrier cannot be identified.
         * @hide
         */
    @Override public java.lang.String getSubscriptionSpecificCarrierName(int subId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Returns carrier id based on MCCMNC only. This will return a MNO carrier id used for fallback
         * check when exact carrier id {@link #getSimCarrierId()} configurations are not found
         *
         * @param isSubscriptionMccMnc. If {@true} it means this is a query for subscription mccmnc
         * {@false} otherwise.
         *
         * @return carrier id from passing mccmnc.
         * @hide
         */
    @Override public int getCarrierIdFromMccMnc(int slotIndex, java.lang.String mccmnc, boolean isSubscriptionMccMnc) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Action set from carrier signalling broadcast receivers to enable/disable metered apns
         * Permissions android.Manifest.permission.MODIFY_PHONE_STATE is required
         * @param subId the subscription ID that this action applies to.
         * @param enabled control enable or disable metered apns.
         * @hide
         */
    @Override public void carrierActionSetMeteredApnsEnabled(int subId, boolean visible) throws android.os.RemoteException
    {
    }
    /**
         * Action set from carrier signalling broadcast receivers to enable/disable radio
         * Permissions android.Manifest.permission.MODIFY_PHONE_STATE is required
         * @param subId the subscription ID that this action applies to.
         * @param enabled control enable or disable radio.
         * @hide
         */
    @Override public void carrierActionSetRadioEnabled(int subId, boolean enabled) throws android.os.RemoteException
    {
    }
    /**
         * Action set from carrier signalling broadcast receivers to start/stop reporting default
         * network conditions.
         * Permissions android.Manifest.permission.MODIFY_PHONE_STATE is required
         * @param subId the subscription ID that this action applies to.
         * @param report control start/stop reporting default network events.
         * @hide
         */
    @Override public void carrierActionReportDefaultNetworkStatus(int subId, boolean report) throws android.os.RemoteException
    {
    }
    /**
         * Action set from carrier signalling broadcast receivers to reset all carrier actions.
         * Permissions android.Manifest.permission.MODIFY_PHONE_STATE is required
         * @param subId the subscription ID that this action applies to.
         * @hide
         */
    @Override public void carrierActionResetAll(int subId) throws android.os.RemoteException
    {
    }
    /**
         * Get aggregated video call data usage since boot.
         * Permissions android.Manifest.permission.READ_NETWORK_USAGE_HISTORY is required.
         *
         * @param perUidStats True if requesting data usage per uid, otherwise overall usage.
         * @return Snapshot of video call data usage
         * @hide
         */
    @Override public android.net.NetworkStats getVtDataUsage(int subId, boolean perUidStats) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Policy control of data connection. Usually used when data limit is passed.
         * @param enabled True if enabling the data, otherwise disabling.
         * @param subId Subscription index
         * @hide
         */
    @Override public void setPolicyDataEnabled(boolean enabled, int subId) throws android.os.RemoteException
    {
    }
    /**
         * Get Client request stats which will contain statistical information
         * on each request made by client.
         * @param callingPackage package making the call.
         * @param subId Subscription index
         * @hide
         */
    @Override public java.util.List<android.telephony.ClientRequestStats> getClientRequestStats(java.lang.String callingPackage, int subid) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Set SIM card power state.
         * @param slotIndex SIM slot id
         * @param state  State of SIM (power down, power up, pass through)
         * @hide
         * */
    @Override public void setSimPowerStateForSlot(int slotIndex, int state) throws android.os.RemoteException
    {
    }
    /**
         * Returns a list of Forbidden PLMNs from the specified SIM App
         * Returns null if the query fails.
         *
         * <p>Requires that the calling app has READ_PRIVILEGED_PHONE_STATE or READ_PHONE_STATE
         *
         * @param subId subscription ID used for authentication
         * @param appType the icc application type, like {@link #APPTYPE_USIM}
         */
    @Override public java.lang.String[] getForbiddenPlmns(int subId, int appType, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Check if phone is in emergency callback mode
         * @return true if phone is in emergency callback mode
         * @param subId the subscription ID that this action applies to.
         * @hide
         */
    @Override public boolean getEmergencyCallbackMode(int subId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Get the most recently available signal strength information.
         *
         * Get the most recent SignalStrength information reported by the modem. Due
         * to power saving this information may not always be current.
         * @param subId Subscription index
         * @return the most recent cached signal strength info from the modem
         * @hide
         */
    @Override public android.telephony.SignalStrength getSignalStrength(int subId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Get the card ID of the default eUICC card. If there is no eUICC, returns
         * {@link #INVALID_CARD_ID}.
         *
         * @param subId subscription ID used for authentication
         * @param callingPackage package making the call
         * @return card ID of the default eUICC card.
         */
    @Override public int getCardIdForDefaultEuicc(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Gets information about currently inserted UICCs and eUICCs.
         * <p>
         * Requires that the calling app has carrier privileges (see {@link #hasCarrierPrivileges}).
         * <p>
         * If the caller has carrier priviliges on any active subscription, then they have permission to
         * get simple information like the card ID ({@link UiccCardInfo#getCardId()}), whether the card
         * is an eUICC ({@link UiccCardInfo#isEuicc()}), and the slot index where the card is inserted
         * ({@link UiccCardInfo#getSlotIndex()}).
         * <p>
         * To get private information such as the EID ({@link UiccCardInfo#getEid()}) or ICCID
         * ({@link UiccCardInfo#getIccId()}), the caller must have carrier priviliges on that specific
         * UICC or eUICC card.
         * <p>
         * See {@link UiccCardInfo} for more details on the kind of information available.
         *
         * @param callingPackage package making the call, used to evaluate carrier privileges
         * @return a list of UiccCardInfo objects, representing information on the currently inserted
         * UICCs and eUICCs. Each UiccCardInfo in the list will have private information filtered out if
         * the caller does not have adequate permissions for that card.
         */
    @Override public java.util.List<android.telephony.UiccCardInfo> getUiccCardsInfo(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Get slot info for all the UICC slots.
         * @return UiccSlotInfo array.
         * @hide
         */
    @Override public android.telephony.UiccSlotInfo[] getUiccSlotsInfo() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Map logicalSlot to physicalSlot, and activate the physicalSlot if it is inactive.
         * @param physicalSlots Index i in the array representing physical slot for phone i. The array
         *        size should be same as getPhoneCount().
         * @return boolean Return true if the switch succeeds, false if the switch fails.
         */
    @Override public boolean switchSlots(int[] physicalSlots) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Sets radio indication update mode. This can be used to control the behavior of indication
         * update from modem to Android frameworks. For example, by default several indication updates
         * are turned off when screen is off, but in some special cases (e.g. carkit is connected but
         * screen is off) we want to turn on those indications even when the screen is off.
         */
    @Override public void setRadioIndicationUpdateMode(int subId, int filters, int mode) throws android.os.RemoteException
    {
    }
    /**
         * Returns whether mobile data roaming is enabled on the subscription with id {@code subId}.
         *
         * @param subId the subscription id
         * @return {@code true} if the data roaming is enabled on this subscription.
         */
    @Override public boolean isDataRoamingEnabled(int subId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Enables/Disables the data roaming on the subscription with id {@code subId}.
         *
         * @param subId the subscription id
         * @param isEnabled {@code true} to enable mobile data roaming, otherwise disable it.
         */
    @Override public void setDataRoamingEnabled(int subId, boolean isEnabled) throws android.os.RemoteException
    {
    }
    /**
         * Gets the roaming mode for the CDMA phone with the subscription id {@code subId}.
         *
         * @param the subscription id.
         * @return the roaming mode for CDMA phone.
         */
    @Override public int getCdmaRoamingMode(int subId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Sets the roaming mode on the CDMA phone with the subscription {@code subId} to the given
         * roaming mode {@code mode}.
         *
         * @param subId the subscription id.
         * @param mode the roaming mode should be set.
         * @return {@code true} if successed.
         */
    @Override public boolean setCdmaRoamingMode(int subId, int mode) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Sets the subscription mode for CDMA phone with the subscription {@code subId} to the given
         * subscription mode {@code mode}.
         *
         * @param subId the subscription id.
         * @param mode the subscription mode should be set.
         * @return {@code true} if successed.
         */
    @Override public boolean setCdmaSubscriptionMode(int subId, int mode) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * A test API to override carrier information including mccmnc, imsi, iccid, gid1, gid2,
         * plmn and spn. This would be handy for, eg, forcing a particular carrier id, carrier's config
         * (also any country or carrier overlays) to be loaded when using a test SIM with a call box.
         */
    @Override public void setCarrierTestOverride(int subId, java.lang.String mccmnc, java.lang.String imsi, java.lang.String iccid, java.lang.String gid1, java.lang.String gid2, java.lang.String plmn, java.lang.String spn, java.lang.String carrierPrivilegeRules, java.lang.String apn) throws android.os.RemoteException
    {
    }
    /**
         * A test API to return installed carrier id list version.
         */
    @Override public int getCarrierIdListVersion(int subId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * A test API to reload the UICC profile.
         * @hide
         */
    @Override public void refreshUiccProfile(int subId) throws android.os.RemoteException
    {
    }
    /**
         * How many modems can have simultaneous data connections.
         * @hide
         */
    @Override public int getNumberOfModemsWithSimultaneousDataConnections(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Return the network selection mode on the subscription with id {@code subId}.
         */
    @Override public int getNetworkSelectionMode(int subId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Return true if the device is in emergency sms mode, false otherwise.
         */
    @Override public boolean isInEmergencySmsMode() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Get a list of SMS apps on a user.
         */
    @Override public java.lang.String[] getSmsApps(int userId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Get the default SMS app on a given user.
         */
    @Override public java.lang.String getDefaultSmsApp(int userId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Set the default SMS app to a given package on a given user.
         */
    @Override public void setDefaultSmsApp(int userId, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    /**
         * Return the modem radio power state for slot index.
         *
         */
    @Override public int getRadioPowerState(int slotIndex, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    // IMS specific AIDL commands, see ImsMmTelManager.java
    /**
         * Adds an IMS registration status callback for the subscription id specified.
         */
    @Override public void registerImsRegistrationCallback(int subId, android.telephony.ims.aidl.IImsRegistrationCallback c) throws android.os.RemoteException
    {
    }
    /**
          * Removes an existing IMS registration status callback for the subscription specified.
          */
    @Override public void unregisterImsRegistrationCallback(int subId, android.telephony.ims.aidl.IImsRegistrationCallback c) throws android.os.RemoteException
    {
    }
    /**
         * Adds an IMS MmTel capabilities callback for the subscription specified.
         */
    @Override public void registerMmTelCapabilityCallback(int subId, android.telephony.ims.aidl.IImsCapabilityCallback c) throws android.os.RemoteException
    {
    }
    /**
         * Removes an existing IMS MmTel capabilities callback for the subscription specified.
         */
    @Override public void unregisterMmTelCapabilityCallback(int subId, android.telephony.ims.aidl.IImsCapabilityCallback c) throws android.os.RemoteException
    {
    }
    /**
         * return true if the IMS MmTel capability for the given registration tech is capable.
         */
    @Override public boolean isCapable(int subId, int capability, int regTech) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * return true if the IMS MmTel capability for the given registration tech is available.
         */
    @Override public boolean isAvailable(int subId, int capability, int regTech) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Returns true if the user's setting for 4G LTE is enabled, for the subscription specified.
         */
    @Override public boolean isAdvancedCallingSettingEnabled(int subId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Modify the user's setting for whether or not 4G LTE is enabled.
         */
    @Override public void setAdvancedCallingSettingEnabled(int subId, boolean isEnabled) throws android.os.RemoteException
    {
    }
    /**
         * return true if the user's setting for VT is enabled for the subscription.
         */
    @Override public boolean isVtSettingEnabled(int subId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Modify the user's setting for whether or not VT is available for the subscrption specified.
         */
    @Override public void setVtSettingEnabled(int subId, boolean isEnabled) throws android.os.RemoteException
    {
    }
    /**
         * return true if the user's setting for whether or not Voice over WiFi is currently enabled.
         */
    @Override public boolean isVoWiFiSettingEnabled(int subId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * sets the user's setting for Voice over WiFi enabled state.
         */
    @Override public void setVoWiFiSettingEnabled(int subId, boolean isEnabled) throws android.os.RemoteException
    {
    }
    /**
         * return true if the user's setting for Voice over WiFi while roaming is enabled.
         */
    @Override public boolean isVoWiFiRoamingSettingEnabled(int subId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Sets the user's preference for whether or not Voice over WiFi is enabled for the current
         * subscription while roaming.
         */
    @Override public void setVoWiFiRoamingSettingEnabled(int subId, boolean isEnabled) throws android.os.RemoteException
    {
    }
    /**
         * Set the Voice over WiFi enabled state, but do not persist the setting.
         */
    @Override public void setVoWiFiNonPersistent(int subId, boolean isCapable, int mode) throws android.os.RemoteException
    {
    }
    /**
         * return the Voice over WiFi mode preference set by the user for the subscription specified.
         */
    @Override public int getVoWiFiModeSetting(int subId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * sets the user's preference for the Voice over WiFi mode for the subscription specified.
         */
    @Override public void setVoWiFiModeSetting(int subId, int mode) throws android.os.RemoteException
    {
    }
    /**
         * return the Voice over WiFi mode preference set by the user for the subscription specified
         * while roaming.
         */
    @Override public int getVoWiFiRoamingModeSetting(int subId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * sets the user's preference for the Voice over WiFi mode for the subscription specified
         * while roaming.
         */
    @Override public void setVoWiFiRoamingModeSetting(int subId, int mode) throws android.os.RemoteException
    {
    }
    /**
         * Modify the user's setting for whether or not RTT is enabled for the subscrption specified.
         */
    @Override public void setRttCapabilitySetting(int subId, boolean isEnabled) throws android.os.RemoteException
    {
    }
    /**
         * return true if TTY over VoLTE is enabled for the subscription specified.
         */
    @Override public boolean isTtyOverVolteEnabled(int subId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Return the emergency number list from all the active subscriptions.
         */
    @Override public java.util.Map getEmergencyNumberList(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Identify if the number is emergency number, based on all the active subscriptions.
         */
    @Override public boolean isEmergencyNumber(java.lang.String number, boolean exactMatch) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Return a list of certs in hex string from loaded carrier privileges access rules.
         */
    @Override public java.util.List<java.lang.String> getCertsFromCarrierPrivilegeAccessRules(int subId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Register an IMS provisioning change callback with Telephony.
         */
    @Override public void registerImsProvisioningChangedCallback(int subId, android.telephony.ims.aidl.IImsConfigCallback callback) throws android.os.RemoteException
    {
    }
    /**
         * unregister an existing IMS provisioning change callback.
         */
    @Override public void unregisterImsProvisioningChangedCallback(int subId, android.telephony.ims.aidl.IImsConfigCallback callback) throws android.os.RemoteException
    {
    }
    /**
         * Set the provisioning status for the IMS MmTel capability using the specified subscription.
         */
    @Override public void setImsProvisioningStatusForCapability(int subId, int capability, int tech, boolean isProvisioned) throws android.os.RemoteException
    {
    }
    /**
         * Get the provisioning status for the IMS MmTel capability specified.
         */
    @Override public boolean getImsProvisioningStatusForCapability(int subId, int capability, int tech) throws android.os.RemoteException
    {
      return false;
    }
    /** Is the capability and tech flagged as provisioned in the cache */
    @Override public boolean isMmTelCapabilityProvisionedInCache(int subId, int capability, int tech) throws android.os.RemoteException
    {
      return false;
    }
    /** Set the provisioning for the capability and tech in the cache */
    @Override public void cacheMmTelCapabilityProvisioning(int subId, int capability, int tech, boolean isProvisioned) throws android.os.RemoteException
    {
    }
    /**
         * Return an integer containing the provisioning value for the specified provisioning key.
         */
    @Override public int getImsProvisioningInt(int subId, int key) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * return a String containing the provisioning value for the provisioning key specified.
         */
    @Override public java.lang.String getImsProvisioningString(int subId, int key) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Set the integer provisioning value for the provisioning key specified.
         */
    @Override public int setImsProvisioningInt(int subId, int key, int value) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Set the String provisioning value for the provisioning key specified.
         */
    @Override public int setImsProvisioningString(int subId, int key, java.lang.String value) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Update Emergency Number List for Test Mode.
         */
    @Override public void updateEmergencyNumberListTestMode(int action, android.telephony.emergency.EmergencyNumber num) throws android.os.RemoteException
    {
    }
    /**
         * Get the full emergency number list for Test Mode.
         */
    @Override public java.util.List<java.lang.String> getEmergencyNumberListTestMode() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Enable or disable a logical modem stack associated with the slotIndex.
         */
    @Override public boolean enableModemForSlot(int slotIndex, boolean enable) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Indicate if the enablement of multi SIM functionality is restricted.
         * @hide
         */
    @Override public void setMultiSimCarrierRestriction(boolean isMultiSimCarrierRestricted) throws android.os.RemoteException
    {
    }
    /**
         * Returns if the usage of multiple SIM cards at the same time is supported.
         *
         * @param callingPackage The package making the call.
         * @return {@link #MULTISIM_ALLOWED} if the device supports multiple SIMs.
         * {@link #MULTISIM_NOT_SUPPORTED_BY_HARDWARE} if the device does not support multiple SIMs.
         * {@link #MULTISIM_NOT_SUPPORTED_BY_CARRIER} in the device supports multiple SIMs, but the
         * functionality is restricted by the carrier.
         */
    @Override public int isMultiSimSupported(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Switch configs to enable multi-sim or switch back to single-sim
         * @hide
         */
    @Override public void switchMultiSimConfig(int numOfSims) throws android.os.RemoteException
    {
    }
    /**
         * Get if altering modems configurations will trigger reboot.
         * @hide
         */
    @Override public boolean doesSwitchMultiSimConfigTriggerReboot(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Get the mapping from logical slots to physical slots.
         */
    @Override public int[] getSlotsMapping() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Get the IRadio HAL Version encoded as 100 * MAJOR_VERSION + MINOR_VERSION or -1 if unknown
         */
    @Override public int getRadioHalVersion() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean isModemEnabledForSlot(int slotIndex, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isDataEnabledForApn(int apnType, int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isApnMetered(int apnType, int subId) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Enqueue a pending sms Consumer, which will answer with the user specified selection for an
         * outgoing SmsManager operation.
         */
    @Override public void enqueueSmsPickResult(java.lang.String callingPackage, com.android.internal.telephony.IIntegerConsumer subIdResult) throws android.os.RemoteException
    {
    }
    /**
         * Returns the MMS user agent.
         */
    @Override public java.lang.String getMmsUserAgent(int subId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Returns the MMS user agent profile URL.
         */
    @Override public java.lang.String getMmsUAProfUrl(int subId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Set allowing mobile data during voice call.
         */
    @Override public boolean setDataAllowedDuringVoiceCall(int subId, boolean allow) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Check whether data is allowed during voice call. Note this is for dual sim device that
         * data might be disabled on non-default data subscription but explicitly turned on by settings.
         */
    @Override public boolean isDataAllowedInVoiceCall(int subId) throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.telephony.ITelephony
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.telephony.ITelephony";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.telephony.ITelephony interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.telephony.ITelephony asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.telephony.ITelephony))) {
        return ((com.android.internal.telephony.ITelephony)iin);
      }
      return new com.android.internal.telephony.ITelephony.Stub.Proxy(obj);
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
        case TRANSACTION_dial:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.dial(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_call:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.call(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isRadioOn:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.isRadioOn(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isRadioOnForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.isRadioOnForSubscriber(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_supplyPin:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.supplyPin(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_supplyPinForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.supplyPinForSubscriber(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_supplyPuk:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.supplyPuk(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_supplyPukForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          boolean _result = this.supplyPukForSubscriber(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_supplyPinReportResult:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int[] _result = this.supplyPinReportResult(_arg0);
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_supplyPinReportResultForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int[] _result = this.supplyPinReportResultForSubscriber(_arg0, _arg1);
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_supplyPukReportResult:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int[] _result = this.supplyPukReportResult(_arg0, _arg1);
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_supplyPukReportResultForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int[] _result = this.supplyPukReportResultForSubscriber(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_handlePinMmi:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.handlePinMmi(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_handleUssdRequest:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.ResultReceiver _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.ResultReceiver.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.handleUssdRequest(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_handlePinMmiForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.handlePinMmiForSubscriber(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_toggleRadioOnOff:
        {
          data.enforceInterface(descriptor);
          this.toggleRadioOnOff();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_toggleRadioOnOffForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.toggleRadioOnOffForSubscriber(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setRadio:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _result = this.setRadio(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setRadioForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          boolean _result = this.setRadioForSubscriber(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setRadioPower:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _result = this.setRadioPower(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_updateServiceLocation:
        {
          data.enforceInterface(descriptor);
          this.updateServiceLocation();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_updateServiceLocationForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.updateServiceLocationForSubscriber(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_enableLocationUpdates:
        {
          data.enforceInterface(descriptor);
          this.enableLocationUpdates();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_enableLocationUpdatesForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.enableLocationUpdatesForSubscriber(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_disableLocationUpdates:
        {
          data.enforceInterface(descriptor);
          this.disableLocationUpdates();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_disableLocationUpdatesForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.disableLocationUpdatesForSubscriber(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_enableDataConnectivity:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.enableDataConnectivity();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_disableDataConnectivity:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.disableDataConnectivity();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isDataConnectivityPossible:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isDataConnectivityPossible(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getCellLocation:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.Bundle _result = this.getCellLocation(_arg0);
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
        case TRANSACTION_getNetworkCountryIsoForPhone:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _result = this.getNetworkCountryIsoForPhone(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getNeighboringCellInfo:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.util.List<android.telephony.NeighboringCellInfo> _result = this.getNeighboringCellInfo(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getCallState:
        {
          data.enforceInterface(descriptor);
          int _result = this.getCallState();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getCallStateForSlot:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getCallStateForSlot(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getDataActivity:
        {
          data.enforceInterface(descriptor);
          int _result = this.getDataActivity();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getDataActivityForSubId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getDataActivityForSubId(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getDataState:
        {
          data.enforceInterface(descriptor);
          int _result = this.getDataState();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getDataStateForSubId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getDataStateForSubId(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getActivePhoneType:
        {
          data.enforceInterface(descriptor);
          int _result = this.getActivePhoneType();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getActivePhoneTypeForSlot:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getActivePhoneTypeForSlot(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getCdmaEriIconIndex:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _result = this.getCdmaEriIconIndex(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getCdmaEriIconIndexForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.getCdmaEriIconIndexForSubscriber(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getCdmaEriIconMode:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _result = this.getCdmaEriIconMode(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getCdmaEriIconModeForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.getCdmaEriIconModeForSubscriber(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getCdmaEriText:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _result = this.getCdmaEriText(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getCdmaEriTextForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _result = this.getCdmaEriTextForSubscriber(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_needsOtaServiceProvisioning:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.needsOtaServiceProvisioning();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setVoiceMailNumber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          boolean _result = this.setVoiceMailNumber(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setVoiceActivationState:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.setVoiceActivationState(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setDataActivationState:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.setDataActivationState(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getVoiceActivationState:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.getVoiceActivationState(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getDataActivationState:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.getDataActivationState(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getVoiceMessageCountForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.getVoiceMessageCountForSubscriber(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_isConcurrentVoiceAndDataAllowed:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isConcurrentVoiceAndDataAllowed(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getVisualVoicemailSettings:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.os.Bundle _result = this.getVisualVoicemailSettings(_arg0, _arg1);
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
        case TRANSACTION_getVisualVoicemailPackageName:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _result = this.getVisualVoicemailPackageName(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_enableVisualVoicemailSmsFilter:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.telephony.VisualVoicemailSmsFilterSettings _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telephony.VisualVoicemailSmsFilterSettings.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.enableVisualVoicemailSmsFilter(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_disableVisualVoicemailSmsFilter:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.disableVisualVoicemailSmsFilter(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_getVisualVoicemailSmsFilterSettings:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.telephony.VisualVoicemailSmsFilterSettings _result = this.getVisualVoicemailSmsFilterSettings(_arg0, _arg1);
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
        case TRANSACTION_getActiveVisualVoicemailSmsFilterSettings:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.VisualVoicemailSmsFilterSettings _result = this.getActiveVisualVoicemailSmsFilterSettings(_arg0);
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
        case TRANSACTION_sendVisualVoicemailSmsForSubscriber:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          java.lang.String _arg4;
          _arg4 = data.readString();
          android.app.PendingIntent _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          this.sendVisualVoicemailSmsForSubscriber(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_sendDialerSpecialCode:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.sendDialerSpecialCode(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getNetworkTypeForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.getNetworkTypeForSubscriber(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getDataNetworkType:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _result = this.getDataNetworkType(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getDataNetworkTypeForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.getDataNetworkTypeForSubscriber(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getVoiceNetworkTypeForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.getVoiceNetworkTypeForSubscriber(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_hasIccCard:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.hasIccCard();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_hasIccCardUsingSlotIndex:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.hasIccCardUsingSlotIndex(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getLteOnCdmaMode:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _result = this.getLteOnCdmaMode(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getLteOnCdmaModeForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.getLteOnCdmaModeForSubscriber(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getAllCellInfo:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.util.List<android.telephony.CellInfo> _result = this.getAllCellInfo(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_requestCellInfoUpdate:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.ICellInfoCallback _arg1;
          _arg1 = android.telephony.ICellInfoCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.requestCellInfoUpdate(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_requestCellInfoUpdateWithWorkSource:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.ICellInfoCallback _arg1;
          _arg1 = android.telephony.ICellInfoCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.os.WorkSource _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.WorkSource.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.requestCellInfoUpdateWithWorkSource(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setCellInfoListRate:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setCellInfoListRate(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_iccOpenLogicalChannelBySlot:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          android.telephony.IccOpenLogicalChannelResponse _result = this.iccOpenLogicalChannelBySlot(_arg0, _arg1, _arg2, _arg3);
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
        case TRANSACTION_iccOpenLogicalChannel:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          android.telephony.IccOpenLogicalChannelResponse _result = this.iccOpenLogicalChannel(_arg0, _arg1, _arg2, _arg3);
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
        case TRANSACTION_iccCloseLogicalChannelBySlot:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.iccCloseLogicalChannelBySlot(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_iccCloseLogicalChannel:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.iccCloseLogicalChannel(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_iccTransmitApduLogicalChannelBySlot:
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
          int _arg4;
          _arg4 = data.readInt();
          int _arg5;
          _arg5 = data.readInt();
          int _arg6;
          _arg6 = data.readInt();
          java.lang.String _arg7;
          _arg7 = data.readString();
          java.lang.String _result = this.iccTransmitApduLogicalChannelBySlot(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_iccTransmitApduLogicalChannel:
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
          int _arg4;
          _arg4 = data.readInt();
          int _arg5;
          _arg5 = data.readInt();
          int _arg6;
          _arg6 = data.readInt();
          java.lang.String _arg7;
          _arg7 = data.readString();
          java.lang.String _result = this.iccTransmitApduLogicalChannel(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_iccTransmitApduBasicChannelBySlot:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          int _arg5;
          _arg5 = data.readInt();
          int _arg6;
          _arg6 = data.readInt();
          java.lang.String _arg7;
          _arg7 = data.readString();
          java.lang.String _result = this.iccTransmitApduBasicChannelBySlot(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_iccTransmitApduBasicChannel:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          int _arg5;
          _arg5 = data.readInt();
          int _arg6;
          _arg6 = data.readInt();
          java.lang.String _arg7;
          _arg7 = data.readString();
          java.lang.String _result = this.iccTransmitApduBasicChannel(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_iccExchangeSimIO:
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
          int _arg4;
          _arg4 = data.readInt();
          int _arg5;
          _arg5 = data.readInt();
          java.lang.String _arg6;
          _arg6 = data.readString();
          byte[] _result = this.iccExchangeSimIO(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          reply.writeNoException();
          reply.writeByteArray(_result);
          return true;
        }
        case TRANSACTION_sendEnvelopeWithStatus:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _result = this.sendEnvelopeWithStatus(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_nvReadItem:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _result = this.nvReadItem(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_nvWriteItem:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.nvWriteItem(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_nvWriteCdmaPrl:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          boolean _result = this.nvWriteCdmaPrl(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_resetModemConfig:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.resetModemConfig(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_rebootModem:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.rebootModem(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getCalculatedPreferredNetworkType:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _result = this.getCalculatedPreferredNetworkType(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getPreferredNetworkType:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getPreferredNetworkType(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getTetherApnRequiredForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.getTetherApnRequiredForSubscriber(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_enableIms:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.enableIms(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_disableIms:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.disableIms(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getMmTelFeatureAndListen:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          com.android.ims.internal.IImsServiceFeatureCallback _arg1;
          _arg1 = com.android.ims.internal.IImsServiceFeatureCallback.Stub.asInterface(data.readStrongBinder());
          android.telephony.ims.aidl.IImsMmTelFeature _result = this.getMmTelFeatureAndListen(_arg0, _arg1);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_getRcsFeatureAndListen:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          com.android.ims.internal.IImsServiceFeatureCallback _arg1;
          _arg1 = com.android.ims.internal.IImsServiceFeatureCallback.Stub.asInterface(data.readStrongBinder());
          android.telephony.ims.aidl.IImsRcsFeature _result = this.getRcsFeatureAndListen(_arg0, _arg1);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_getImsRegistration:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          android.telephony.ims.aidl.IImsRegistration _result = this.getImsRegistration(_arg0, _arg1);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_getImsConfig:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          android.telephony.ims.aidl.IImsConfig _result = this.getImsConfig(_arg0, _arg1);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_setImsService:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.lang.String _arg2;
          _arg2 = data.readString();
          boolean _result = this.setImsService(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getImsService:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.lang.String _result = this.getImsService(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_setNetworkSelectionModeAutomatic:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setNetworkSelectionModeAutomatic(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getCellNetworkScanResults:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          com.android.internal.telephony.CellNetworkScanResult _result = this.getCellNetworkScanResults(_arg0, _arg1);
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
        case TRANSACTION_requestNetworkScan:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.NetworkScanRequest _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telephony.NetworkScanRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.os.Messenger _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Messenger.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.os.IBinder _arg3;
          _arg3 = data.readStrongBinder();
          java.lang.String _arg4;
          _arg4 = data.readString();
          int _result = this.requestNetworkScan(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_stopNetworkScan:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.stopNetworkScan(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setNetworkSelectionModeManual:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          com.android.internal.telephony.OperatorInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = com.android.internal.telephony.OperatorInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          boolean _result = this.setNetworkSelectionModeManual(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setPreferredNetworkType:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.setPreferredNetworkType(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setUserDataEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setUserDataEnabled(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getDataEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.getDataEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isUserDataEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isUserDataEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isDataEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isDataEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isManualNetworkSelectionAllowed:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isManualNetworkSelectionAllowed(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getPcscfAddress:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String[] _result = this.getPcscfAddress(_arg0, _arg1);
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_setImsRegistrationState:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setImsRegistrationState(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getCdmaMdn:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _result = this.getCdmaMdn(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getCdmaMin:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _result = this.getCdmaMin(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_requestNumberVerification:
        {
          data.enforceInterface(descriptor);
          android.telephony.PhoneNumberRange _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.PhoneNumberRange.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          long _arg1;
          _arg1 = data.readLong();
          com.android.internal.telephony.INumberVerificationCallback _arg2;
          _arg2 = com.android.internal.telephony.INumberVerificationCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.requestNumberVerification(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getCarrierPrivilegeStatus:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getCarrierPrivilegeStatus(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getCarrierPrivilegeStatusForUid:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.getCarrierPrivilegeStatusForUid(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_checkCarrierPrivilegesForPackage:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.checkCarrierPrivilegesForPackage(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_checkCarrierPrivilegesForPackageAnyPhone:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _result = this.checkCarrierPrivilegesForPackageAnyPhone(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getCarrierPackageNamesForIntentAndPhone:
        {
          data.enforceInterface(descriptor);
          android.content.Intent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          java.util.List<java.lang.String> _result = this.getCarrierPackageNamesForIntentAndPhone(_arg0, _arg1);
          reply.writeNoException();
          reply.writeStringList(_result);
          return true;
        }
        case TRANSACTION_setLine1NumberForDisplayForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          boolean _result = this.setLine1NumberForDisplayForSubscriber(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getLine1NumberForDisplay:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _result = this.getLine1NumberForDisplay(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getLine1AlphaTagForDisplay:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _result = this.getLine1AlphaTagForDisplay(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getMergedSubscriberIds:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String[] _result = this.getMergedSubscriberIds(_arg0, _arg1);
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_getMergedSubscriberIdsFromGroup:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String[] _result = this.getMergedSubscriberIdsFromGroup(_arg0, _arg1);
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_setOperatorBrandOverride:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.setOperatorBrandOverride(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setRoamingOverride:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<java.lang.String> _arg1;
          _arg1 = data.createStringArrayList();
          java.util.List<java.lang.String> _arg2;
          _arg2 = data.createStringArrayList();
          java.util.List<java.lang.String> _arg3;
          _arg3 = data.createStringArrayList();
          java.util.List<java.lang.String> _arg4;
          _arg4 = data.createStringArrayList();
          boolean _result = this.setRoamingOverride(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_invokeOemRilRequestRaw:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          byte[] _arg1;
          int _arg1_length = data.readInt();
          if ((_arg1_length<0)) {
            _arg1 = null;
          }
          else {
            _arg1 = new byte[_arg1_length];
          }
          int _result = this.invokeOemRilRequestRaw(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          reply.writeByteArray(_arg1);
          return true;
        }
        case TRANSACTION_needMobileRadioShutdown:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.needMobileRadioShutdown();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_shutdownMobileRadios:
        {
          data.enforceInterface(descriptor);
          this.shutdownMobileRadios();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setRadioCapability:
        {
          data.enforceInterface(descriptor);
          android.telephony.RadioAccessFamily[] _arg0;
          _arg0 = data.createTypedArray(android.telephony.RadioAccessFamily.CREATOR);
          this.setRadioCapability(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getRadioAccessFamily:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.getRadioAccessFamily(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_enableVideoCalling:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.enableVideoCalling(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isVideoCallingEnabled:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.isVideoCallingEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_canChangeDtmfToneLength:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.canChangeDtmfToneLength(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isWorldPhone:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.isWorldPhone(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isTtyModeSupported:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isTtyModeSupported();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isRttSupported:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isRttSupported(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isHearingAidCompatibilitySupported:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isHearingAidCompatibilitySupported();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isImsRegistered:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isImsRegistered(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isWifiCallingAvailable:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isWifiCallingAvailable(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isVideoTelephonyAvailable:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isVideoTelephonyAvailable(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getImsRegTechnologyForMmTel:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getImsRegTechnologyForMmTel(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getDeviceId:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _result = this.getDeviceId(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getImeiForSlot:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _result = this.getImeiForSlot(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getTypeAllocationCodeForSlot:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _result = this.getTypeAllocationCodeForSlot(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getMeidForSlot:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _result = this.getMeidForSlot(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getManufacturerCodeForSlot:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _result = this.getManufacturerCodeForSlot(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getDeviceSoftwareVersionForSlot:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _result = this.getDeviceSoftwareVersionForSlot(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getSubIdForPhoneAccount:
        {
          data.enforceInterface(descriptor);
          android.telecom.PhoneAccount _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telecom.PhoneAccount.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _result = this.getSubIdForPhoneAccount(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getPhoneAccountHandleForSubscriptionId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telecom.PhoneAccountHandle _result = this.getPhoneAccountHandleForSubscriptionId(_arg0);
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
        case TRANSACTION_factoryReset:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.factoryReset(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getSimLocaleForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _result = this.getSimLocaleForSubscriber(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_requestModemActivityInfo:
        {
          data.enforceInterface(descriptor);
          android.os.ResultReceiver _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ResultReceiver.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.requestModemActivityInfo(_arg0);
          return true;
        }
        case TRANSACTION_getServiceStateForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.telephony.ServiceState _result = this.getServiceStateForSubscriber(_arg0, _arg1);
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
        case TRANSACTION_getVoicemailRingtoneUri:
        {
          data.enforceInterface(descriptor);
          android.telecom.PhoneAccountHandle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telecom.PhoneAccountHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.net.Uri _result = this.getVoicemailRingtoneUri(_arg0);
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
        case TRANSACTION_setVoicemailRingtoneUri:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.PhoneAccountHandle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.PhoneAccountHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.net.Uri _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.setVoicemailRingtoneUri(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isVoicemailVibrationEnabled:
        {
          data.enforceInterface(descriptor);
          android.telecom.PhoneAccountHandle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telecom.PhoneAccountHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _result = this.isVoicemailVibrationEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setVoicemailVibrationEnabled:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.PhoneAccountHandle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.PhoneAccountHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.setVoicemailVibrationEnabled(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getPackagesWithCarrierPrivileges:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<java.lang.String> _result = this.getPackagesWithCarrierPrivileges(_arg0);
          reply.writeNoException();
          reply.writeStringList(_result);
          return true;
        }
        case TRANSACTION_getPackagesWithCarrierPrivilegesForAllPhones:
        {
          data.enforceInterface(descriptor);
          java.util.List<java.lang.String> _result = this.getPackagesWithCarrierPrivilegesForAllPhones();
          reply.writeNoException();
          reply.writeStringList(_result);
          return true;
        }
        case TRANSACTION_getAidForAppType:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _result = this.getAidForAppType(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getEsn:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _result = this.getEsn(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getCdmaPrlVersion:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _result = this.getCdmaPrlVersion(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getTelephonyHistograms:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.telephony.TelephonyHistogram> _result = this.getTelephonyHistograms();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_setAllowedCarriers:
        {
          data.enforceInterface(descriptor);
          android.telephony.CarrierRestrictionRules _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.CarrierRestrictionRules.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _result = this.setAllowedCarriers(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getAllowedCarriers:
        {
          data.enforceInterface(descriptor);
          android.telephony.CarrierRestrictionRules _result = this.getAllowedCarriers();
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
        case TRANSACTION_getSubscriptionCarrierId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getSubscriptionCarrierId(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getSubscriptionCarrierName:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _result = this.getSubscriptionCarrierName(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getSubscriptionSpecificCarrierId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getSubscriptionSpecificCarrierId(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getSubscriptionSpecificCarrierName:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _result = this.getSubscriptionSpecificCarrierName(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getCarrierIdFromMccMnc:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          int _result = this.getCarrierIdFromMccMnc(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_carrierActionSetMeteredApnsEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.carrierActionSetMeteredApnsEnabled(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_carrierActionSetRadioEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.carrierActionSetRadioEnabled(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_carrierActionReportDefaultNetworkStatus:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.carrierActionReportDefaultNetworkStatus(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_carrierActionResetAll:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.carrierActionResetAll(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getVtDataUsage:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          android.net.NetworkStats _result = this.getVtDataUsage(_arg0, _arg1);
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
        case TRANSACTION_setPolicyDataEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          this.setPolicyDataEnabled(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getClientRequestStats:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          java.util.List<android.telephony.ClientRequestStats> _result = this.getClientRequestStats(_arg0, _arg1);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_setSimPowerStateForSlot:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.setSimPowerStateForSlot(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getForbiddenPlmns:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String[] _result = this.getForbiddenPlmns(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_getEmergencyCallbackMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.getEmergencyCallbackMode(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getSignalStrength:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.SignalStrength _result = this.getSignalStrength(_arg0);
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
        case TRANSACTION_getCardIdForDefaultEuicc:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.getCardIdForDefaultEuicc(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getUiccCardsInfo:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.util.List<android.telephony.UiccCardInfo> _result = this.getUiccCardsInfo(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getUiccSlotsInfo:
        {
          data.enforceInterface(descriptor);
          android.telephony.UiccSlotInfo[] _result = this.getUiccSlotsInfo();
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_switchSlots:
        {
          data.enforceInterface(descriptor);
          int[] _arg0;
          _arg0 = data.createIntArray();
          boolean _result = this.switchSlots(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setRadioIndicationUpdateMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.setRadioIndicationUpdateMode(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isDataRoamingEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isDataRoamingEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setDataRoamingEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setDataRoamingEnabled(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getCdmaRoamingMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getCdmaRoamingMode(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setCdmaRoamingMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.setCdmaRoamingMode(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setCdmaSubscriptionMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.setCdmaSubscriptionMode(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setCarrierTestOverride:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String _arg3;
          _arg3 = data.readString();
          java.lang.String _arg4;
          _arg4 = data.readString();
          java.lang.String _arg5;
          _arg5 = data.readString();
          java.lang.String _arg6;
          _arg6 = data.readString();
          java.lang.String _arg7;
          _arg7 = data.readString();
          java.lang.String _arg8;
          _arg8 = data.readString();
          java.lang.String _arg9;
          _arg9 = data.readString();
          this.setCarrierTestOverride(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getCarrierIdListVersion:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getCarrierIdListVersion(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_refreshUiccProfile:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.refreshUiccProfile(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getNumberOfModemsWithSimultaneousDataConnections:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.getNumberOfModemsWithSimultaneousDataConnections(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getNetworkSelectionMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getNetworkSelectionMode(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_isInEmergencySmsMode:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isInEmergencySmsMode();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getSmsApps:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String[] _result = this.getSmsApps(_arg0);
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_getDefaultSmsApp:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _result = this.getDefaultSmsApp(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_setDefaultSmsApp:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.setDefaultSmsApp(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getRadioPowerState:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.getRadioPowerState(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_registerImsRegistrationCallback:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.ims.aidl.IImsRegistrationCallback _arg1;
          _arg1 = android.telephony.ims.aidl.IImsRegistrationCallback.Stub.asInterface(data.readStrongBinder());
          this.registerImsRegistrationCallback(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterImsRegistrationCallback:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.ims.aidl.IImsRegistrationCallback _arg1;
          _arg1 = android.telephony.ims.aidl.IImsRegistrationCallback.Stub.asInterface(data.readStrongBinder());
          this.unregisterImsRegistrationCallback(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerMmTelCapabilityCallback:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.ims.aidl.IImsCapabilityCallback _arg1;
          _arg1 = android.telephony.ims.aidl.IImsCapabilityCallback.Stub.asInterface(data.readStrongBinder());
          this.registerMmTelCapabilityCallback(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterMmTelCapabilityCallback:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.ims.aidl.IImsCapabilityCallback _arg1;
          _arg1 = android.telephony.ims.aidl.IImsCapabilityCallback.Stub.asInterface(data.readStrongBinder());
          this.unregisterMmTelCapabilityCallback(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isCapable:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          boolean _result = this.isCapable(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isAvailable:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          boolean _result = this.isAvailable(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isAdvancedCallingSettingEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isAdvancedCallingSettingEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setAdvancedCallingSettingEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setAdvancedCallingSettingEnabled(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isVtSettingEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isVtSettingEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setVtSettingEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setVtSettingEnabled(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isVoWiFiSettingEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isVoWiFiSettingEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setVoWiFiSettingEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setVoWiFiSettingEnabled(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isVoWiFiRoamingSettingEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isVoWiFiRoamingSettingEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setVoWiFiRoamingSettingEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setVoWiFiRoamingSettingEnabled(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setVoWiFiNonPersistent:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          int _arg2;
          _arg2 = data.readInt();
          this.setVoWiFiNonPersistent(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getVoWiFiModeSetting:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getVoWiFiModeSetting(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setVoWiFiModeSetting:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.setVoWiFiModeSetting(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getVoWiFiRoamingModeSetting:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getVoWiFiRoamingModeSetting(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setVoWiFiRoamingModeSetting:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.setVoWiFiRoamingModeSetting(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setRttCapabilitySetting:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setRttCapabilitySetting(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isTtyOverVolteEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isTtyOverVolteEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getEmergencyNumberList:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.util.Map _result = this.getEmergencyNumberList(_arg0);
          reply.writeNoException();
          reply.writeMap(_result);
          return true;
        }
        case TRANSACTION_isEmergencyNumber:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          boolean _result = this.isEmergencyNumber(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getCertsFromCarrierPrivilegeAccessRules:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<java.lang.String> _result = this.getCertsFromCarrierPrivilegeAccessRules(_arg0);
          reply.writeNoException();
          reply.writeStringList(_result);
          return true;
        }
        case TRANSACTION_registerImsProvisioningChangedCallback:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.ims.aidl.IImsConfigCallback _arg1;
          _arg1 = android.telephony.ims.aidl.IImsConfigCallback.Stub.asInterface(data.readStrongBinder());
          this.registerImsProvisioningChangedCallback(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterImsProvisioningChangedCallback:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.ims.aidl.IImsConfigCallback _arg1;
          _arg1 = android.telephony.ims.aidl.IImsConfigCallback.Stub.asInterface(data.readStrongBinder());
          this.unregisterImsProvisioningChangedCallback(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setImsProvisioningStatusForCapability:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          this.setImsProvisioningStatusForCapability(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getImsProvisioningStatusForCapability:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          boolean _result = this.getImsProvisioningStatusForCapability(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isMmTelCapabilityProvisionedInCache:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          boolean _result = this.isMmTelCapabilityProvisionedInCache(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_cacheMmTelCapabilityProvisioning:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          this.cacheMmTelCapabilityProvisioning(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getImsProvisioningInt:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.getImsProvisioningInt(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getImsProvisioningString:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _result = this.getImsProvisioningString(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_setImsProvisioningInt:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _result = this.setImsProvisioningInt(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setImsProvisioningString:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _result = this.setImsProvisioningString(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_updateEmergencyNumberListTestMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.emergency.EmergencyNumber _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telephony.emergency.EmergencyNumber.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.updateEmergencyNumberListTestMode(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getEmergencyNumberListTestMode:
        {
          data.enforceInterface(descriptor);
          java.util.List<java.lang.String> _result = this.getEmergencyNumberListTestMode();
          reply.writeNoException();
          reply.writeStringList(_result);
          return true;
        }
        case TRANSACTION_enableModemForSlot:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          boolean _result = this.enableModemForSlot(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setMultiSimCarrierRestriction:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setMultiSimCarrierRestriction(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isMultiSimSupported:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _result = this.isMultiSimSupported(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_switchMultiSimConfig:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.switchMultiSimConfig(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_doesSwitchMultiSimConfigTriggerReboot:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.doesSwitchMultiSimConfigTriggerReboot(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getSlotsMapping:
        {
          data.enforceInterface(descriptor);
          int[] _result = this.getSlotsMapping();
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_getRadioHalVersion:
        {
          data.enforceInterface(descriptor);
          int _result = this.getRadioHalVersion();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_isModemEnabledForSlot:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.isModemEnabledForSlot(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isDataEnabledForApn:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          boolean _result = this.isDataEnabledForApn(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isApnMetered:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.isApnMetered(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_enqueueSmsPickResult:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          com.android.internal.telephony.IIntegerConsumer _arg1;
          _arg1 = com.android.internal.telephony.IIntegerConsumer.Stub.asInterface(data.readStrongBinder());
          this.enqueueSmsPickResult(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_getMmsUserAgent:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _result = this.getMmsUserAgent(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getMmsUAProfUrl:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _result = this.getMmsUAProfUrl(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_setDataAllowedDuringVoiceCall:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          boolean _result = this.setDataAllowedDuringVoiceCall(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isDataAllowedInVoiceCall:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isDataAllowedInVoiceCall(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.telephony.ITelephony
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
           * Dial a number. This doesn't place the call. It displays
           * the Dialer screen.
           * @param number the number to be dialed. If null, this
           * would display the Dialer screen with no number pre-filled.
           */
      @Override public void dial(java.lang.String number) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(number);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dial, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dial(number);
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
           * Place a call to the specified number.
           * @param callingPackage The package making the call.
           * @param number the number to be called.
           */
      @Override public void call(java.lang.String callingPackage, java.lang.String number) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(number);
          boolean _status = mRemote.transact(Stub.TRANSACTION_call, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().call(callingPackage, number);
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
           * Check to see if the radio is on or not.
           * @param callingPackage the name of the package making the call.
           * @return returns true if the radio is on.
           */
      @Override public boolean isRadioOn(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isRadioOn, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isRadioOn(callingPackage);
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
      /**
           * Check to see if the radio is on or not on particular subId.
           * @param subId user preferred subId.
           * @param callingPackage the name of the package making the call.
           * @return returns true if the radio is on.
           */
      @Override public boolean isRadioOnForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isRadioOnForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isRadioOnForSubscriber(subId, callingPackage);
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
      /**
           * Supply a pin to unlock the SIM.  Blocks until a result is determined.
           * @param pin The pin to check.
           * @return whether the operation was a success.
           */
      @Override public boolean supplyPin(java.lang.String pin) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(pin);
          boolean _status = mRemote.transact(Stub.TRANSACTION_supplyPin, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().supplyPin(pin);
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
      /**
           * Supply a pin to unlock the SIM for particular subId.
           * Blocks until a result is determined.
           * @param pin The pin to check.
           * @param subId user preferred subId.
           * @return whether the operation was a success.
           */
      @Override public boolean supplyPinForSubscriber(int subId, java.lang.String pin) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(pin);
          boolean _status = mRemote.transact(Stub.TRANSACTION_supplyPinForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().supplyPinForSubscriber(subId, pin);
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
      /**
           * Supply puk to unlock the SIM and set SIM pin to new pin.
           *  Blocks until a result is determined.
           * @param puk The puk to check.
           *        pin The new pin to be set in SIM
           * @return whether the operation was a success.
           */
      @Override public boolean supplyPuk(java.lang.String puk, java.lang.String pin) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(puk);
          _data.writeString(pin);
          boolean _status = mRemote.transact(Stub.TRANSACTION_supplyPuk, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().supplyPuk(puk, pin);
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
      /**
           * Supply puk to unlock the SIM and set SIM pin to new pin.
           *  Blocks until a result is determined.
           * @param puk The puk to check.
           *        pin The new pin to be set in SIM
           * @param subId user preferred subId.
           * @return whether the operation was a success.
           */
      @Override public boolean supplyPukForSubscriber(int subId, java.lang.String puk, java.lang.String pin) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(puk);
          _data.writeString(pin);
          boolean _status = mRemote.transact(Stub.TRANSACTION_supplyPukForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().supplyPukForSubscriber(subId, puk, pin);
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
      /**
           * Supply a pin to unlock the SIM.  Blocks until a result is determined.
           * Returns a specific success/error code.
           * @param pin The pin to check.
           * @return retValue[0] = Phone.PIN_RESULT_SUCCESS on success. Otherwise error code
           *         retValue[1] = number of attempts remaining if known otherwise -1
           */
      @Override public int[] supplyPinReportResult(java.lang.String pin) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(pin);
          boolean _status = mRemote.transact(Stub.TRANSACTION_supplyPinReportResult, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().supplyPinReportResult(pin);
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
      /**
           * Supply a pin to unlock the SIM.  Blocks until a result is determined.
           * Returns a specific success/error code.
           * @param pin The pin to check.
           * @return retValue[0] = Phone.PIN_RESULT_SUCCESS on success. Otherwise error code
           *         retValue[1] = number of attempts remaining if known otherwise -1
           */
      @Override public int[] supplyPinReportResultForSubscriber(int subId, java.lang.String pin) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(pin);
          boolean _status = mRemote.transact(Stub.TRANSACTION_supplyPinReportResultForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().supplyPinReportResultForSubscriber(subId, pin);
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
      /**
           * Supply puk to unlock the SIM and set SIM pin to new pin.
           * Blocks until a result is determined.
           * Returns a specific success/error code
           * @param puk The puk to check
           *        pin The pin to check.
           * @return retValue[0] = Phone.PIN_RESULT_SUCCESS on success. Otherwise error code
           *         retValue[1] = number of attempts remaining if known otherwise -1
           */
      @Override public int[] supplyPukReportResult(java.lang.String puk, java.lang.String pin) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(puk);
          _data.writeString(pin);
          boolean _status = mRemote.transact(Stub.TRANSACTION_supplyPukReportResult, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().supplyPukReportResult(puk, pin);
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
      /**
           * Supply puk to unlock the SIM and set SIM pin to new pin.
           * Blocks until a result is determined.
           * Returns a specific success/error code
           * @param puk The puk to check
           *        pin The pin to check.
           * @return retValue[0] = Phone.PIN_RESULT_SUCCESS on success. Otherwise error code
           *         retValue[1] = number of attempts remaining if known otherwise -1
           */
      @Override public int[] supplyPukReportResultForSubscriber(int subId, java.lang.String puk, java.lang.String pin) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(puk);
          _data.writeString(pin);
          boolean _status = mRemote.transact(Stub.TRANSACTION_supplyPukReportResultForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().supplyPukReportResultForSubscriber(subId, puk, pin);
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
      /**
           * Handles PIN MMI commands (PIN/PIN2/PUK/PUK2), which are initiated
           * without SEND (so <code>dial</code> is not appropriate).
           *
           * @param dialString the MMI command to be executed.
           * @return true if MMI command is executed.
           */
      @Override public boolean handlePinMmi(java.lang.String dialString) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(dialString);
          boolean _status = mRemote.transact(Stub.TRANSACTION_handlePinMmi, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().handlePinMmi(dialString);
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
      /**
           * Handles USSD commands.
           *
           * @param subId The subscription to use.
           * @param ussdRequest the USSD command to be executed.
           * @param wrappedCallback receives a callback result.
           */
      @Override public void handleUssdRequest(int subId, java.lang.String ussdRequest, android.os.ResultReceiver wrappedCallback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(ussdRequest);
          if ((wrappedCallback!=null)) {
            _data.writeInt(1);
            wrappedCallback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_handleUssdRequest, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().handleUssdRequest(subId, ussdRequest, wrappedCallback);
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
           * Handles PIN MMI commands (PIN/PIN2/PUK/PUK2), which are initiated
           * without SEND (so <code>dial</code> is not appropriate) for
           * a particular subId.
           * @param dialString the MMI command to be executed.
           * @param subId user preferred subId.
           * @return true if MMI command is executed.
           */
      @Override public boolean handlePinMmiForSubscriber(int subId, java.lang.String dialString) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(dialString);
          boolean _status = mRemote.transact(Stub.TRANSACTION_handlePinMmiForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().handlePinMmiForSubscriber(subId, dialString);
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
      /**
           * Toggles the radio on or off.
           */
      @Override public void toggleRadioOnOff() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_toggleRadioOnOff, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().toggleRadioOnOff();
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
           * Toggles the radio on or off on particular subId.
           * @param subId user preferred subId.
           */
      @Override public void toggleRadioOnOffForSubscriber(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_toggleRadioOnOffForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().toggleRadioOnOffForSubscriber(subId);
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
           * Set the radio to on or off
           */
      @Override public boolean setRadio(boolean turnOn) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((turnOn)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRadio, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setRadio(turnOn);
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
      /**
           * Set the radio to on or off on particular subId.
           * @param subId user preferred subId.
           */
      @Override public boolean setRadioForSubscriber(int subId, boolean turnOn) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(((turnOn)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRadioForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setRadioForSubscriber(subId, turnOn);
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
      /**
           * Set the radio to on or off unconditionally
           */
      @Override public boolean setRadioPower(boolean turnOn) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((turnOn)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRadioPower, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setRadioPower(turnOn);
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
      /**
           * Request to update location information in service state
           */
      @Override public void updateServiceLocation() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateServiceLocation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateServiceLocation();
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
           * Request to update location information for a subscrition in service state
           * @param subId user preferred subId.
           */
      @Override public void updateServiceLocationForSubscriber(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateServiceLocationForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateServiceLocationForSubscriber(subId);
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
           * Enable location update notifications.
           */
      @Override public void enableLocationUpdates() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableLocationUpdates, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enableLocationUpdates();
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
           * Enable location update notifications.
           * @param subId user preferred subId.
           */
      @Override public void enableLocationUpdatesForSubscriber(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableLocationUpdatesForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enableLocationUpdatesForSubscriber(subId);
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
           * Disable location update notifications.
           */
      @Override public void disableLocationUpdates() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableLocationUpdates, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableLocationUpdates();
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
           * Disable location update notifications.
           * @param subId user preferred subId.
           */
      @Override public void disableLocationUpdatesForSubscriber(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableLocationUpdatesForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableLocationUpdatesForSubscriber(subId);
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
           * Allow mobile data connections.
           */
      @Override public boolean enableDataConnectivity() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableDataConnectivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().enableDataConnectivity();
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
      /**
           * Disallow mobile data connections.
           */
      @Override public boolean disableDataConnectivity() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableDataConnectivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().disableDataConnectivity();
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
      /**
           * Report whether data connectivity is possible.
           */
      @Override public boolean isDataConnectivityPossible(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isDataConnectivityPossible, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isDataConnectivityPossible(subId);
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
      @Override public android.os.Bundle getCellLocation(java.lang.String callingPkg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.Bundle _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPkg);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCellLocation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCellLocation(callingPkg);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.Bundle.CREATOR.createFromParcel(_reply);
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
           * Returns the ISO country code equivalent of the current registered
           * operator's MCC (Mobile Country Code).
           * @see android.telephony.TelephonyManager#getNetworkCountryIso
           */
      @Override public java.lang.String getNetworkCountryIsoForPhone(int phoneId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(phoneId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNetworkCountryIsoForPhone, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNetworkCountryIsoForPhone(phoneId);
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
      /**
           * Returns the neighboring cell information of the device.
           */
      @Override public java.util.List<android.telephony.NeighboringCellInfo> getNeighboringCellInfo(java.lang.String callingPkg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.telephony.NeighboringCellInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPkg);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNeighboringCellInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNeighboringCellInfo(callingPkg);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.telephony.NeighboringCellInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int getCallState() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCallState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCallState();
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
           * Returns the call state for a slot.
           */
      @Override public int getCallStateForSlot(int slotIndex) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotIndex);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCallStateForSlot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCallStateForSlot(slotIndex);
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
           * Replaced by getDataActivityForSubId.
           */
      @Override public int getDataActivity() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDataActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDataActivity();
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
           * Returns a constant indicating the type of activity on a data connection
           * (cellular).
           *
           * @see #DATA_ACTIVITY_NONE
           * @see #DATA_ACTIVITY_IN
           * @see #DATA_ACTIVITY_OUT
           * @see #DATA_ACTIVITY_INOUT
           * @see #DATA_ACTIVITY_DORMANT
           */
      @Override public int getDataActivityForSubId(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDataActivityForSubId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDataActivityForSubId(subId);
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
           * Replaced by getDataStateForSubId.
           */
      @Override public int getDataState() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDataState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDataState();
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
           * Returns a constant indicating the current data connection state
           * (cellular).
           *
           * @see #DATA_DISCONNECTED
           * @see #DATA_CONNECTING
           * @see #DATA_CONNECTED
           * @see #DATA_SUSPENDED
           */
      @Override public int getDataStateForSubId(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDataStateForSubId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDataStateForSubId(subId);
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
           * Returns the current active phone type as integer.
           * Returns TelephonyManager.PHONE_TYPE_CDMA if RILConstants.CDMA_PHONE
           * and TelephonyManager.PHONE_TYPE_GSM if RILConstants.GSM_PHONE
           */
      @Override public int getActivePhoneType() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActivePhoneType, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActivePhoneType();
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
           * Returns the current active phone type as integer for particular slot.
           * Returns TelephonyManager.PHONE_TYPE_CDMA if RILConstants.CDMA_PHONE
           * and TelephonyManager.PHONE_TYPE_GSM if RILConstants.GSM_PHONE
           * @param slotIndex - slot to query.
           */
      @Override public int getActivePhoneTypeForSlot(int slotIndex) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotIndex);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActivePhoneTypeForSlot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActivePhoneTypeForSlot(slotIndex);
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
           * Returns the CDMA ERI icon index to display
           * @param callingPackage package making the call.
           */
      @Override public int getCdmaEriIconIndex(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCdmaEriIconIndex, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCdmaEriIconIndex(callingPackage);
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
           * Returns the CDMA ERI icon index to display on particular subId.
           * @param subId user preferred subId.
           * @param callingPackage package making the call.
           */
      @Override public int getCdmaEriIconIndexForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCdmaEriIconIndexForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCdmaEriIconIndexForSubscriber(subId, callingPackage);
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
           * Returns the CDMA ERI icon mode,
           * 0 - ON
           * 1 - FLASHING
           * @param callingPackage package making the call.
           */
      @Override public int getCdmaEriIconMode(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCdmaEriIconMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCdmaEriIconMode(callingPackage);
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
           * Returns the CDMA ERI icon mode on particular subId,
           * 0 - ON
           * 1 - FLASHING
           * @param subId user preferred subId.
           * @param callingPackage package making the call.
           */
      @Override public int getCdmaEriIconModeForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCdmaEriIconModeForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCdmaEriIconModeForSubscriber(subId, callingPackage);
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
           * Returns the CDMA ERI text,
           * @param callingPackage package making the call.
           */
      @Override public java.lang.String getCdmaEriText(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCdmaEriText, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCdmaEriText(callingPackage);
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
      /**
           * Returns the CDMA ERI text for particular subId,
           * @param subId user preferred subId.
           * @param callingPackage package making the call.
           */
      @Override public java.lang.String getCdmaEriTextForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCdmaEriTextForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCdmaEriTextForSubscriber(subId, callingPackage);
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
      /**
           * Returns true if OTA service provisioning needs to run.
           * Only relevant on some technologies, others will always
           * return false.
           */
      @Override public boolean needsOtaServiceProvisioning() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_needsOtaServiceProvisioning, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().needsOtaServiceProvisioning();
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
      /**
           * Sets the voicemail number for a particular subscriber.
           */
      @Override public boolean setVoiceMailNumber(int subId, java.lang.String alphaTag, java.lang.String number) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(alphaTag);
          _data.writeString(number);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVoiceMailNumber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setVoiceMailNumber(subId, alphaTag, number);
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
      /**
            * Sets the voice activation state for a particular subscriber.
            */
      @Override public void setVoiceActivationState(int subId, int activationState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(activationState);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVoiceActivationState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVoiceActivationState(subId, activationState);
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
            * Sets the data activation state for a particular subscriber.
            */
      @Override public void setDataActivationState(int subId, int activationState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(activationState);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDataActivationState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDataActivationState(subId, activationState);
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
            * Returns the voice activation state for a particular subscriber.
            * @param subId user preferred sub
            * @param callingPackage package queries voice activation state
            */
      @Override public int getVoiceActivationState(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getVoiceActivationState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getVoiceActivationState(subId, callingPackage);
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
            * Returns the data activation state for a particular subscriber.
            * @param subId user preferred sub
            * @param callingPackage package queris data activation state
            */
      @Override public int getDataActivationState(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDataActivationState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDataActivationState(subId, callingPackage);
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
           * Returns the unread count of voicemails for a subId.
           * @param subId user preferred subId.
           * Returns the unread count of voicemails
           */
      @Override public int getVoiceMessageCountForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getVoiceMessageCountForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getVoiceMessageCountForSubscriber(subId, callingPackage);
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
            * Returns true if current state supports both voice and data
            * simultaneously. This can change based on location or network condition.
            */
      @Override public boolean isConcurrentVoiceAndDataAllowed(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isConcurrentVoiceAndDataAllowed, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isConcurrentVoiceAndDataAllowed(subId);
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
      @Override public android.os.Bundle getVisualVoicemailSettings(java.lang.String callingPackage, int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.Bundle _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getVisualVoicemailSettings, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getVisualVoicemailSettings(callingPackage, subId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.Bundle.CREATOR.createFromParcel(_reply);
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
      @Override public java.lang.String getVisualVoicemailPackageName(java.lang.String callingPackage, int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getVisualVoicemailPackageName, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getVisualVoicemailPackageName(callingPackage, subId);
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
      // Not oneway, caller needs to make sure the vaule is set before receiving a SMS

      @Override public void enableVisualVoicemailSmsFilter(java.lang.String callingPackage, int subId, android.telephony.VisualVoicemailSmsFilterSettings settings) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(subId);
          if ((settings!=null)) {
            _data.writeInt(1);
            settings.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableVisualVoicemailSmsFilter, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enableVisualVoicemailSmsFilter(callingPackage, subId, settings);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void disableVisualVoicemailSmsFilter(java.lang.String callingPackage, int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableVisualVoicemailSmsFilter, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableVisualVoicemailSmsFilter(callingPackage, subId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Get settings set by the calling package

      @Override public android.telephony.VisualVoicemailSmsFilterSettings getVisualVoicemailSmsFilterSettings(java.lang.String callingPackage, int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.VisualVoicemailSmsFilterSettings _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getVisualVoicemailSmsFilterSettings, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getVisualVoicemailSmsFilterSettings(callingPackage, subId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telephony.VisualVoicemailSmsFilterSettings.CREATOR.createFromParcel(_reply);
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
           *  Get settings set by the current default dialer, Internal use only.
           *  Requires READ_PRIVILEGED_PHONE_STATE permission.
           */
      @Override public android.telephony.VisualVoicemailSmsFilterSettings getActiveVisualVoicemailSmsFilterSettings(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.VisualVoicemailSmsFilterSettings _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActiveVisualVoicemailSmsFilterSettings, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActiveVisualVoicemailSmsFilterSettings(subId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telephony.VisualVoicemailSmsFilterSettings.CREATOR.createFromParcel(_reply);
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
           * Send a visual voicemail SMS. Internal use only.
           * Requires caller to be the default dialer and have SEND_SMS permission
           */
      @Override public void sendVisualVoicemailSmsForSubscriber(java.lang.String callingPackage, int subId, java.lang.String number, int port, java.lang.String text, android.app.PendingIntent sentIntent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(subId);
          _data.writeString(number);
          _data.writeInt(port);
          _data.writeString(text);
          if ((sentIntent!=null)) {
            _data.writeInt(1);
            sentIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendVisualVoicemailSmsForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendVisualVoicemailSmsForSubscriber(callingPackage, subId, number, port, text, sentIntent);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Send the special dialer code. The IPC caller must be the current default dialer.

      @Override public void sendDialerSpecialCode(java.lang.String callingPackageName, java.lang.String inputCode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackageName);
          _data.writeString(inputCode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendDialerSpecialCode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendDialerSpecialCode(callingPackageName, inputCode);
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
           * Returns the network type of a subId.
           * @param subId user preferred subId.
           * @param callingPackage package making the call.
           */
      @Override public int getNetworkTypeForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNetworkTypeForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNetworkTypeForSubscriber(subId, callingPackage);
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
           * Returns the network type for data transmission
           * @param callingPackage package making the call.
           */
      @Override public int getDataNetworkType(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDataNetworkType, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDataNetworkType(callingPackage);
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
           * Returns the data network type of a subId
           * @param subId user preferred subId.
           * @param callingPackage package making the call.
           */
      @Override public int getDataNetworkTypeForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDataNetworkTypeForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDataNetworkTypeForSubscriber(subId, callingPackage);
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
            * Returns the voice network type of a subId
            * @param subId user preferred subId.
            * @param callingPackage package making the call.
            * Returns the network type
            */
      @Override public int getVoiceNetworkTypeForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getVoiceNetworkTypeForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getVoiceNetworkTypeForSubscriber(subId, callingPackage);
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
           * Return true if an ICC card is present
           */
      @Override public boolean hasIccCard() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasIccCard, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasIccCard();
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
      /**
           * Return true if an ICC card is present for a subId.
           * @param slotIndex user preferred slotIndex.
           * Return true if an ICC card is present
           */
      @Override public boolean hasIccCardUsingSlotIndex(int slotIndex) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotIndex);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasIccCardUsingSlotIndex, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasIccCardUsingSlotIndex(slotIndex);
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
      /**
           * Return if the current radio is LTE on CDMA. This
           * is a tri-state return value as for a period of time
           * the mode may be unknown.
           *
           * @param callingPackage the name of the calling package
           * @return {@link Phone#LTE_ON_CDMA_UNKNOWN}, {@link Phone#LTE_ON_CDMA_FALSE}
           * or {@link PHone#LTE_ON_CDMA_TRUE}
           */
      @Override public int getLteOnCdmaMode(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLteOnCdmaMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLteOnCdmaMode(callingPackage);
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
           * Return if the current radio is LTE on CDMA. This
           * is a tri-state return value as for a period of time
           * the mode may be unknown.
           *
           * @param callingPackage the name of the calling package
           * @return {@link Phone#LTE_ON_CDMA_UNKNOWN}, {@link Phone#LTE_ON_CDMA_FALSE}
           * or {@link PHone#LTE_ON_CDMA_TRUE}
           */
      @Override public int getLteOnCdmaModeForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLteOnCdmaModeForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLteOnCdmaModeForSubscriber(subId, callingPackage);
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
           * Returns all observed cell information of the device.
           */
      @Override public java.util.List<android.telephony.CellInfo> getAllCellInfo(java.lang.String callingPkg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.telephony.CellInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPkg);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAllCellInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAllCellInfo(callingPkg);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.telephony.CellInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Request a cell information update for the specified subscription,
           * reported via the CellInfoCallback.
           */
      @Override public void requestCellInfoUpdate(int subId, android.telephony.ICellInfoCallback cb, java.lang.String callingPkg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeStrongBinder((((cb!=null))?(cb.asBinder()):(null)));
          _data.writeString(callingPkg);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestCellInfoUpdate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestCellInfoUpdate(subId, cb, callingPkg);
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
           * Request a cell information update for the specified subscription,
           * reported via the CellInfoCallback.
           *
           * @param workSource the requestor to whom the power consumption for this should be attributed.
           */
      @Override public void requestCellInfoUpdateWithWorkSource(int subId, android.telephony.ICellInfoCallback cb, java.lang.String callingPkg, android.os.WorkSource ws) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeStrongBinder((((cb!=null))?(cb.asBinder()):(null)));
          _data.writeString(callingPkg);
          if ((ws!=null)) {
            _data.writeInt(1);
            ws.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestCellInfoUpdateWithWorkSource, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestCellInfoUpdateWithWorkSource(subId, cb, callingPkg, ws);
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
           * Sets minimum time in milli-seconds between onCellInfoChanged
           */
      @Override public void setCellInfoListRate(int rateInMillis) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(rateInMillis);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setCellInfoListRate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setCellInfoListRate(rateInMillis);
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
           * Opens a logical channel to the ICC card using the physical slot index.
           *
           * Input parameters equivalent to TS 27.007 AT+CCHO command.
           *
           * @param slotIndex The physical slot index of the target ICC card
           * @param callingPackage the name of the package making the call.
           * @param AID Application id. See ETSI 102.221 and 101.220.
           * @param p2 P2 parameter (described in ISO 7816-4).
           * @return an IccOpenLogicalChannelResponse object.
           */
      @Override public android.telephony.IccOpenLogicalChannelResponse iccOpenLogicalChannelBySlot(int slotIndex, java.lang.String callingPackage, java.lang.String AID, int p2) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.IccOpenLogicalChannelResponse _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotIndex);
          _data.writeString(callingPackage);
          _data.writeString(AID);
          _data.writeInt(p2);
          boolean _status = mRemote.transact(Stub.TRANSACTION_iccOpenLogicalChannelBySlot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().iccOpenLogicalChannelBySlot(slotIndex, callingPackage, AID, p2);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telephony.IccOpenLogicalChannelResponse.CREATOR.createFromParcel(_reply);
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
           * Opens a logical channel to the ICC card.
           *
           * Input parameters equivalent to TS 27.007 AT+CCHO command.
           *
           * @param subId The subscription to use.
           * @param callingPackage the name of the package making the call.
           * @param AID Application id. See ETSI 102.221 and 101.220.
           * @param p2 P2 parameter (described in ISO 7816-4).
           * @return an IccOpenLogicalChannelResponse object.
           */
      @Override public android.telephony.IccOpenLogicalChannelResponse iccOpenLogicalChannel(int subId, java.lang.String callingPackage, java.lang.String AID, int p2) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.IccOpenLogicalChannelResponse _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          _data.writeString(AID);
          _data.writeInt(p2);
          boolean _status = mRemote.transact(Stub.TRANSACTION_iccOpenLogicalChannel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().iccOpenLogicalChannel(subId, callingPackage, AID, p2);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telephony.IccOpenLogicalChannelResponse.CREATOR.createFromParcel(_reply);
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
           * Closes a previously opened logical channel to the ICC card using the physical slot index.
           *
           * Input parameters equivalent to TS 27.007 AT+CCHC command.
           *
           * @param slotIndex The physical slot index of the target ICC card
           * @param channel is the channel id to be closed as returned by a
           *            successful iccOpenLogicalChannel.
           * @return true if the channel was closed successfully.
           */
      @Override public boolean iccCloseLogicalChannelBySlot(int slotIndex, int channel) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotIndex);
          _data.writeInt(channel);
          boolean _status = mRemote.transact(Stub.TRANSACTION_iccCloseLogicalChannelBySlot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().iccCloseLogicalChannelBySlot(slotIndex, channel);
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
      /**
           * Closes a previously opened logical channel to the ICC card.
           *
           * Input parameters equivalent to TS 27.007 AT+CCHC command.
           *
           * @param subId The subscription to use.
           * @param channel is the channel id to be closed as returned by a
           *            successful iccOpenLogicalChannel.
           * @return true if the channel was closed successfully.
           */
      @Override public boolean iccCloseLogicalChannel(int subId, int channel) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(channel);
          boolean _status = mRemote.transact(Stub.TRANSACTION_iccCloseLogicalChannel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().iccCloseLogicalChannel(subId, channel);
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
      /**
           * Transmit an APDU to the ICC card over a logical channel using the physical slot index.
           *
           * Input parameters equivalent to TS 27.007 AT+CGLA command.
           *
           * @param slotIndex The physical slot index of the target ICC card
           * @param channel is the channel id to be closed as returned by a
           *            successful iccOpenLogicalChannel.
           * @param cla Class of the APDU command.
           * @param instruction Instruction of the APDU command.
           * @param p1 P1 value of the APDU command.
           * @param p2 P2 value of the APDU command.
           * @param p3 P3 value of the APDU command. If p3 is negative a 4 byte APDU
           *            is sent to the SIM.
           * @param data Data to be sent with the APDU.
           * @return The APDU response from the ICC card with the status appended at
           *            the end.
           */
      @Override public java.lang.String iccTransmitApduLogicalChannelBySlot(int slotIndex, int channel, int cla, int instruction, int p1, int p2, int p3, java.lang.String data) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotIndex);
          _data.writeInt(channel);
          _data.writeInt(cla);
          _data.writeInt(instruction);
          _data.writeInt(p1);
          _data.writeInt(p2);
          _data.writeInt(p3);
          _data.writeString(data);
          boolean _status = mRemote.transact(Stub.TRANSACTION_iccTransmitApduLogicalChannelBySlot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().iccTransmitApduLogicalChannelBySlot(slotIndex, channel, cla, instruction, p1, p2, p3, data);
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
      /**
           * Transmit an APDU to the ICC card over a logical channel.
           *
           * Input parameters equivalent to TS 27.007 AT+CGLA command.
           *
           * @param subId The subscription to use.
           * @param channel is the channel id to be closed as returned by a
           *            successful iccOpenLogicalChannel.
           * @param cla Class of the APDU command.
           * @param instruction Instruction of the APDU command.
           * @param p1 P1 value of the APDU command.
           * @param p2 P2 value of the APDU command.
           * @param p3 P3 value of the APDU command. If p3 is negative a 4 byte APDU
           *            is sent to the SIM.
           * @param data Data to be sent with the APDU.
           * @return The APDU response from the ICC card with the status appended at
           *            the end.
           */
      @Override public java.lang.String iccTransmitApduLogicalChannel(int subId, int channel, int cla, int instruction, int p1, int p2, int p3, java.lang.String data) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(channel);
          _data.writeInt(cla);
          _data.writeInt(instruction);
          _data.writeInt(p1);
          _data.writeInt(p2);
          _data.writeInt(p3);
          _data.writeString(data);
          boolean _status = mRemote.transact(Stub.TRANSACTION_iccTransmitApduLogicalChannel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().iccTransmitApduLogicalChannel(subId, channel, cla, instruction, p1, p2, p3, data);
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
      /**
           * Transmit an APDU to the ICC card over the basic channel using the physical slot index.
           *
           * Input parameters equivalent to TS 27.007 AT+CSIM command.
           *
           * @param slotIndex The physical slot index of the target ICC card
           * @param callingPackage the name of the package making the call.
           * @param cla Class of the APDU command.
           * @param instruction Instruction of the APDU command.
           * @param p1 P1 value of the APDU command.
           * @param p2 P2 value of the APDU command.
           * @param p3 P3 value of the APDU command. If p3 is negative a 4 byte APDU
           *            is sent to the SIM.
           * @param data Data to be sent with the APDU.
           * @return The APDU response from the ICC card with the status appended at
           *            the end.
           */
      @Override public java.lang.String iccTransmitApduBasicChannelBySlot(int slotIndex, java.lang.String callingPackage, int cla, int instruction, int p1, int p2, int p3, java.lang.String data) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotIndex);
          _data.writeString(callingPackage);
          _data.writeInt(cla);
          _data.writeInt(instruction);
          _data.writeInt(p1);
          _data.writeInt(p2);
          _data.writeInt(p3);
          _data.writeString(data);
          boolean _status = mRemote.transact(Stub.TRANSACTION_iccTransmitApduBasicChannelBySlot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().iccTransmitApduBasicChannelBySlot(slotIndex, callingPackage, cla, instruction, p1, p2, p3, data);
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
      /**
           * Transmit an APDU to the ICC card over the basic channel.
           *
           * Input parameters equivalent to TS 27.007 AT+CSIM command.
           *
           * @param subId The subscription to use.
           * @param callingPackage the name of the package making the call.
           * @param cla Class of the APDU command.
           * @param instruction Instruction of the APDU command.
           * @param p1 P1 value of the APDU command.
           * @param p2 P2 value of the APDU command.
           * @param p3 P3 value of the APDU command. If p3 is negative a 4 byte APDU
           *            is sent to the SIM.
           * @param data Data to be sent with the APDU.
           * @return The APDU response from the ICC card with the status appended at
           *            the end.
           */
      @Override public java.lang.String iccTransmitApduBasicChannel(int subId, java.lang.String callingPackage, int cla, int instruction, int p1, int p2, int p3, java.lang.String data) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          _data.writeInt(cla);
          _data.writeInt(instruction);
          _data.writeInt(p1);
          _data.writeInt(p2);
          _data.writeInt(p3);
          _data.writeString(data);
          boolean _status = mRemote.transact(Stub.TRANSACTION_iccTransmitApduBasicChannel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().iccTransmitApduBasicChannel(subId, callingPackage, cla, instruction, p1, p2, p3, data);
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
      /**
           * Returns the response APDU for a command APDU sent through SIM_IO.
           *
           * @param subId The subscription to use.
           * @param fileID
           * @param command
           * @param p1 P1 value of the APDU command.
           * @param p2 P2 value of the APDU command.
           * @param p3 P3 value of the APDU command.
           * @param filePath
           * @return The APDU response.
           */
      @Override public byte[] iccExchangeSimIO(int subId, int fileID, int command, int p1, int p2, int p3, java.lang.String filePath) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        byte[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(fileID);
          _data.writeInt(command);
          _data.writeInt(p1);
          _data.writeInt(p2);
          _data.writeInt(p3);
          _data.writeString(filePath);
          boolean _status = mRemote.transact(Stub.TRANSACTION_iccExchangeSimIO, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().iccExchangeSimIO(subId, fileID, command, p1, p2, p3, filePath);
          }
          _reply.readException();
          _result = _reply.createByteArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Send ENVELOPE to the SIM and returns the response.
           *
           * @param subId The subscription to use.
           * @param contents  String containing SAT/USAT response in hexadecimal
           *                  format starting with command tag. See TS 102 223 for
           *                  details.
           * @return The APDU response from the ICC card, with the last 4 bytes
           *         being the status word. If the command fails, returns an empty
           *         string.
           */
      @Override public java.lang.String sendEnvelopeWithStatus(int subId, java.lang.String content) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(content);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendEnvelopeWithStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().sendEnvelopeWithStatus(subId, content);
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
      /**
           * Read one of the NV items defined in {@link RadioNVItems} / {@code ril_nv_items.h}.
           * Used for device configuration by some CDMA operators.
           *
           * @param itemID the ID of the item to read.
           * @return the NV item as a String, or null on any failure.
           */
      @Override public java.lang.String nvReadItem(int itemID) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(itemID);
          boolean _status = mRemote.transact(Stub.TRANSACTION_nvReadItem, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().nvReadItem(itemID);
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
      /**
           * Write one of the NV items defined in {@link RadioNVItems} / {@code ril_nv_items.h}.
           * Used for device configuration by some CDMA operators.
           *
           * @param itemID the ID of the item to read.
           * @param itemValue the value to write, as a String.
           * @return true on success; false on any failure.
           */
      @Override public boolean nvWriteItem(int itemID, java.lang.String itemValue) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(itemID);
          _data.writeString(itemValue);
          boolean _status = mRemote.transact(Stub.TRANSACTION_nvWriteItem, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().nvWriteItem(itemID, itemValue);
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
      /**
           * Update the CDMA Preferred Roaming List (PRL) in the radio NV storage.
           * Used for device configuration by some CDMA operators.
           *
           * @param preferredRoamingList byte array containing the new PRL.
           * @return true on success; false on any failure.
           */
      @Override public boolean nvWriteCdmaPrl(byte[] preferredRoamingList) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(preferredRoamingList);
          boolean _status = mRemote.transact(Stub.TRANSACTION_nvWriteCdmaPrl, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().nvWriteCdmaPrl(preferredRoamingList);
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
      /**
           * Rollback modem configurations to factory default except some config which are in whitelist.
           * Used for device configuration by some CDMA operators.
           *
           * <p>Requires Permission:
           * {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE} or that the calling
           * app has carrier privileges (see {@link #hasCarrierPrivileges}).
           *
           * @param slotIndex - device slot.
           * @return {@code true} on success; {@code false} on any failure.
           */
      @Override public boolean resetModemConfig(int slotIndex) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotIndex);
          boolean _status = mRemote.transact(Stub.TRANSACTION_resetModemConfig, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().resetModemConfig(slotIndex);
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
      /**
           * Generate a radio modem reset. Used for device configuration by some CDMA operators.
           * Different than {@link #setRadioPower(boolean)}, modem reboot will power down sim card.
           *
           * <p>Requires Permission:
           * {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE} or that the calling
           * app has carrier privileges (see {@link #hasCarrierPrivileges}).
           *
           * @param slotIndex - device slot.
           * @return {@code true} on success; {@code false} on any failure.
           */
      @Override public boolean rebootModem(int slotIndex) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotIndex);
          boolean _status = mRemote.transact(Stub.TRANSACTION_rebootModem, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().rebootModem(slotIndex);
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
      /*
           * Get the calculated preferred network type.
           * Used for device configuration by some CDMA operators.
           * @param callingPackage The package making the call.
           *
           * @return the calculated preferred network type, defined in RILConstants.java.
           */
      @Override public int getCalculatedPreferredNetworkType(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCalculatedPreferredNetworkType, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCalculatedPreferredNetworkType(callingPackage);
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
      /*
           * Get the preferred network type.
           * Used for device configuration by some CDMA operators.
           *
           * @param subId the id of the subscription to query.
           * @return the preferred network type, defined in RILConstants.java.
           */
      @Override public int getPreferredNetworkType(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPreferredNetworkType, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPreferredNetworkType(subId);
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
           * Check whether DUN APN is required for tethering with subId.
           *
           * @param subId the id of the subscription to require tethering.
           * @return {@code true} if DUN APN is required for tethering.
           * @hide
           */
      @Override public boolean getTetherApnRequiredForSubscriber(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTetherApnRequiredForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTetherApnRequiredForSubscriber(subId);
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
      /**
          * Enables framework IMS and triggers IMS Registration.
          */
      @Override public void enableIms(int slotId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableIms, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enableIms(slotId);
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
          * Disables framework IMS and triggers IMS deregistration.
          */
      @Override public void disableIms(int slotId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableIms, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableIms(slotId);
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
           *  Get IImsMmTelFeature binder from ImsResolver that corresponds to the subId and MMTel feature
           *  as well as registering the MmTelFeature for callbacks using the IImsServiceFeatureCallback
           *  interface.
           */
      @Override public android.telephony.ims.aidl.IImsMmTelFeature getMmTelFeatureAndListen(int slotId, com.android.ims.internal.IImsServiceFeatureCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.ims.aidl.IImsMmTelFeature _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMmTelFeatureAndListen, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMmTelFeatureAndListen(slotId, callback);
          }
          _reply.readException();
          _result = android.telephony.ims.aidl.IImsMmTelFeature.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           *  Get IImsRcsFeature binder from ImsResolver that corresponds to the subId and RCS feature
           *  as well as registering the RcsFeature for callbacks using the IImsServiceFeatureCallback
           *  interface.
           */
      @Override public android.telephony.ims.aidl.IImsRcsFeature getRcsFeatureAndListen(int slotId, com.android.ims.internal.IImsServiceFeatureCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.ims.aidl.IImsRcsFeature _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRcsFeatureAndListen, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRcsFeatureAndListen(slotId, callback);
          }
          _reply.readException();
          _result = android.telephony.ims.aidl.IImsRcsFeature.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
          * Returns the IImsRegistration associated with the slot and feature specified.
          */
      @Override public android.telephony.ims.aidl.IImsRegistration getImsRegistration(int slotId, int feature) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.ims.aidl.IImsRegistration _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeInt(feature);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getImsRegistration, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getImsRegistration(slotId, feature);
          }
          _reply.readException();
          _result = android.telephony.ims.aidl.IImsRegistration.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
          * Returns the IImsConfig associated with the slot and feature specified.
          */
      @Override public android.telephony.ims.aidl.IImsConfig getImsConfig(int slotId, int feature) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.ims.aidl.IImsConfig _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeInt(feature);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getImsConfig, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getImsConfig(slotId, feature);
          }
          _reply.readException();
          _result = android.telephony.ims.aidl.IImsConfig.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
          *  @return true if the ImsService to bind to for the slot id specified was set, false otherwise.
          */
      @Override public boolean setImsService(int slotId, boolean isCarrierImsService, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeInt(((isCarrierImsService)?(1):(0)));
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setImsService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setImsService(slotId, isCarrierImsService, packageName);
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
      /**
          * @return the package name of the carrier/device ImsService associated with this slot.
          */
      @Override public java.lang.String getImsService(int slotId, boolean isCarrierImsService) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotId);
          _data.writeInt(((isCarrierImsService)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getImsService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getImsService(slotId, isCarrierImsService);
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
      /**
           * Set the network selection mode to automatic.
           *
           * @param subId the id of the subscription to update.
           */
      @Override public void setNetworkSelectionModeAutomatic(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setNetworkSelectionModeAutomatic, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setNetworkSelectionModeAutomatic(subId);
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
           * Perform a radio scan and return the list of avialble networks.
           *
           * @param subId the id of the subscription.
           * @return CellNetworkScanResult containing status of scan and networks.
           */
      @Override public com.android.internal.telephony.CellNetworkScanResult getCellNetworkScanResults(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.internal.telephony.CellNetworkScanResult _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCellNetworkScanResults, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCellNetworkScanResults(subId, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = com.android.internal.telephony.CellNetworkScanResult.CREATOR.createFromParcel(_reply);
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
           * Perform a radio network scan and return the id of this scan.
           *
           * @param subId the id of the subscription.
           * @param request Defines all the configs for network scan.
           * @param messenger Callback messages will be sent using this messenger.
           * @param binder the binder object instantiated in TelephonyManager.
           * @param callingPackage the calling package
           * @return An id for this scan.
           */
      @Override public int requestNetworkScan(int subId, android.telephony.NetworkScanRequest request, android.os.Messenger messenger, android.os.IBinder binder, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((messenger!=null)) {
            _data.writeInt(1);
            messenger.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder(binder);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestNetworkScan, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().requestNetworkScan(subId, request, messenger, binder, callingPackage);
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
           * Stop an existing radio network scan.
           *
           * @param subId the id of the subscription.
           * @param scanId The id of the scan that is going to be stopped.
           */
      @Override public void stopNetworkScan(int subId, int scanId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(scanId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopNetworkScan, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopNetworkScan(subId, scanId);
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
           * Ask the radio to connect to the input network and change selection mode to manual.
           *
           * @param subId the id of the subscription.
           * @param operatorInfo the operator inforamtion, included the PLMN, long name and short name of
           * the operator to attach to.
           * @param persistSelection whether the selection will persist until reboot. If true, only allows
           * attaching to the selected PLMN until reboot; otherwise, attach to the chosen PLMN and resume
           * normal network selection next time.
           * @return {@code true} on success; {@code true} on any failure.
           */
      @Override public boolean setNetworkSelectionModeManual(int subId, com.android.internal.telephony.OperatorInfo operatorInfo, boolean persisSelection) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          if ((operatorInfo!=null)) {
            _data.writeInt(1);
            operatorInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((persisSelection)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setNetworkSelectionModeManual, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setNetworkSelectionModeManual(subId, operatorInfo, persisSelection);
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
      /**
           * Set the preferred network type.
           * Used for device configuration by some CDMA operators.
           *
           * @param subId the id of the subscription to update.
           * @param networkType the preferred network type, defined in RILConstants.java.
           * @return true on success; false on any failure.
           */
      @Override public boolean setPreferredNetworkType(int subId, int networkType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(networkType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPreferredNetworkType, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setPreferredNetworkType(subId, networkType);
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
      /**
           * User enable/disable Mobile Data.
           *
           * @param enable true to turn on, else false
           */
      @Override public void setUserDataEnabled(int subId, boolean enable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(((enable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setUserDataEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setUserDataEnabled(subId, enable);
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
           * Get the user enabled state of Mobile Data.
           *
           * TODO: remove and use isUserDataEnabled.
           * This can't be removed now because some vendor codes
           * calls through ITelephony directly while they should
           * use TelephonyManager.
           *
           * @return true on enabled
           */
      @Override public boolean getDataEnabled(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDataEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDataEnabled(subId);
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
      /**
           * Get the user enabled state of Mobile Data.
           *
           * @return true on enabled
           */
      @Override public boolean isUserDataEnabled(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isUserDataEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isUserDataEnabled(subId);
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
      /**
           * Get the overall enabled state of Mobile Data.
           *
           * @return true on enabled
           */
      @Override public boolean isDataEnabled(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isDataEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isDataEnabled(subId);
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
      /**
           * Checks if manual network selection is allowed.
           *
           * @return {@code true} if manual network selection is allowed, otherwise return {@code false}.
           */
      @Override public boolean isManualNetworkSelectionAllowed(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isManualNetworkSelectionAllowed, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isManualNetworkSelectionAllowed(subId);
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
      /**
           * Get P-CSCF address from PCO after data connection is established or modified.
           * @param apnType the apnType, "ims" for IMS APN, "emergency" for EMERGENCY APN
           * @param callingPackage The package making the call.
           */
      @Override public java.lang.String[] getPcscfAddress(java.lang.String apnType, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(apnType);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPcscfAddress, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPcscfAddress(apnType, callingPackage);
          }
          _reply.readException();
          _result = _reply.createStringArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Set IMS registration state
           */
      @Override public void setImsRegistrationState(boolean registered) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((registered)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setImsRegistrationState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setImsRegistrationState(registered);
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
           * Return MDN string for CDMA phone.
           * @param subId user preferred subId.
           */
      @Override public java.lang.String getCdmaMdn(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCdmaMdn, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCdmaMdn(subId);
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
      /**
           * Return MIN string for CDMA phone.
           * @param subId user preferred subId.
           */
      @Override public java.lang.String getCdmaMin(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCdmaMin, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCdmaMin(subId);
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
      /**
           * Request that the next incoming call from a number matching {@code range} be intercepted.
           * @param range The range of phone numbers the caller expects a phone call from.
           * @param timeoutMillis The amount of time to wait for such a call, or
           *                      {@link #MAX_NUMBER_VERIFICATION_TIMEOUT_MILLIS}, whichever is lesser.
           * @param callback the callback aidl
           * @param callingPackage the calling package name.
           */
      @Override public void requestNumberVerification(android.telephony.PhoneNumberRange range, long timeoutMillis, com.android.internal.telephony.INumberVerificationCallback callback, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((range!=null)) {
            _data.writeInt(1);
            range.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeLong(timeoutMillis);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestNumberVerification, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestNumberVerification(range, timeoutMillis, callback, callingPackage);
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
           * Has the calling application been granted special privileges by the carrier.
           *
           * If any of the packages in the calling UID has carrier privileges, the
           * call will return true. This access is granted by the owner of the UICC
           * card and does not depend on the registered carrier.
           *
           * TODO: Add a link to documentation.
           *
           * @param subId The subscription to use.
           * @return carrier privilege status defined in TelephonyManager.
           */
      @Override public int getCarrierPrivilegeStatus(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCarrierPrivilegeStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCarrierPrivilegeStatus(subId);
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
           * Similar to above, but check for the given uid.
           */
      @Override public int getCarrierPrivilegeStatusForUid(int subId, int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCarrierPrivilegeStatusForUid, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCarrierPrivilegeStatusForUid(subId, uid);
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
           * Similar to above, but check for the package whose name is pkgName.
           */
      @Override public int checkCarrierPrivilegesForPackage(int subId, java.lang.String pkgName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(pkgName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_checkCarrierPrivilegesForPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().checkCarrierPrivilegesForPackage(subId, pkgName);
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
           * Similar to above, but check across all phones.
           */
      @Override public int checkCarrierPrivilegesForPackageAnyPhone(java.lang.String pkgName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(pkgName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_checkCarrierPrivilegesForPackageAnyPhone, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().checkCarrierPrivilegesForPackageAnyPhone(pkgName);
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
           * Returns list of the package names of the carrier apps that should handle the input intent
           * and have carrier privileges for the given phoneId.
           *
           * @param intent Intent that will be sent.
           * @param phoneId The phoneId on which the carrier app has carrier privileges.
           * @return list of carrier app package names that can handle the intent on phoneId.
           *         Returns null if there is an error and an empty list if there
           *         are no matching packages.
           */
      @Override public java.util.List<java.lang.String> getCarrierPackageNamesForIntentAndPhone(android.content.Intent intent, int phoneId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<java.lang.String> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(phoneId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCarrierPackageNamesForIntentAndPhone, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCarrierPackageNamesForIntentAndPhone(intent, phoneId);
          }
          _reply.readException();
          _result = _reply.createStringArrayList();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Set the line 1 phone number string and its alphatag for the current ICCID
           * for display purpose only, for example, displayed in Phone Status. It won't
           * change the actual MSISDN/MDN. To unset alphatag or number, pass in a null
           * value.
           *
           * @param subId the subscriber that the alphatag and dialing number belongs to.
           * @param alphaTag alpha-tagging of the dailing nubmer
           * @param number The dialing number
           * @return true if the operation was executed correctly.
           */
      @Override public boolean setLine1NumberForDisplayForSubscriber(int subId, java.lang.String alphaTag, java.lang.String number) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(alphaTag);
          _data.writeString(number);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setLine1NumberForDisplayForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setLine1NumberForDisplayForSubscriber(subId, alphaTag, number);
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
      /**
           * Returns the displayed dialing number string if it was set previously via
           * {@link #setLine1NumberForDisplay}. Otherwise returns null.
           *
           * @param subId whose dialing number for line 1 is returned.
           * @param callingPackage The package making the call.
           * @return the displayed dialing number if set, or null if not set.
           */
      @Override public java.lang.String getLine1NumberForDisplay(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLine1NumberForDisplay, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLine1NumberForDisplay(subId, callingPackage);
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
      /**
           * Returns the displayed alphatag of the dialing number if it was set
           * previously via {@link #setLine1NumberForDisplay}. Otherwise returns null.
           *
           * @param subId whose alphatag associated with line 1 is returned.
           * @param callingPackage The package making the call.
           * @return the displayed alphatag of the dialing number if set, or null if
           *         not set.
           */
      @Override public java.lang.String getLine1AlphaTagForDisplay(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLine1AlphaTagForDisplay, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLine1AlphaTagForDisplay(subId, callingPackage);
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
      /**
           * Return the set of subscriber IDs that should be considered "merged together" for data usage
           * purposes. This is commonly {@code null} to indicate no merging is required. Any returned
           * subscribers are sorted in a deterministic order.
           * <p>
           * The returned set of subscriber IDs will include the subscriber ID corresponding to this
           * TelephonyManager's subId.
           *
           * @hide
           */
      @Override public java.lang.String[] getMergedSubscriberIds(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMergedSubscriberIds, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMergedSubscriberIds(subId, callingPackage);
          }
          _reply.readException();
          _result = _reply.createStringArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * @hide
           */
      @Override public java.lang.String[] getMergedSubscriberIdsFromGroup(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMergedSubscriberIdsFromGroup, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMergedSubscriberIdsFromGroup(subId, callingPackage);
          }
          _reply.readException();
          _result = _reply.createStringArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Override the operator branding for the current ICCID.
           *
           * Once set, whenever the SIM is present in the device, the service
           * provider name (SPN) and the operator name will both be replaced by the
           * brand value input. To unset the value, the same function should be
           * called with a null brand value.
           *
           * <p>Requires Permission:
           *   {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}
           *  or has to be carrier app - see #hasCarrierPrivileges.
           *
           * @param subId The subscription to use.
           * @param brand The brand name to display/set.
           * @return true if the operation was executed correctly.
           */
      @Override public boolean setOperatorBrandOverride(int subId, java.lang.String brand) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(brand);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setOperatorBrandOverride, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setOperatorBrandOverride(subId, brand);
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
      /**
           * Override the roaming indicator for the current ICCID.
           *
           * Using this call, the carrier app (see #hasCarrierPrivileges) can override
           * the platform's notion of a network operator being considered roaming or not.
           * The change only affects the ICCID that was active when this call was made.
           *
           * If null is passed as any of the input, the corresponding value is deleted.
           *
           * <p>Requires that the caller have carrier privilege. See #hasCarrierPrivileges.
           *
           * @param subId for which the roaming overrides apply.
           * @param gsmRoamingList - List of MCCMNCs to be considered roaming for 3GPP RATs.
           * @param gsmNonRoamingList - List of MCCMNCs to be considered not roaming for 3GPP RATs.
           * @param cdmaRoamingList - List of SIDs to be considered roaming for 3GPP2 RATs.
           * @param cdmaNonRoamingList - List of SIDs to be considered not roaming for 3GPP2 RATs.
           * @return true if the operation was executed correctly.
           */
      @Override public boolean setRoamingOverride(int subId, java.util.List<java.lang.String> gsmRoamingList, java.util.List<java.lang.String> gsmNonRoamingList, java.util.List<java.lang.String> cdmaRoamingList, java.util.List<java.lang.String> cdmaNonRoamingList) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeStringList(gsmRoamingList);
          _data.writeStringList(gsmNonRoamingList);
          _data.writeStringList(cdmaRoamingList);
          _data.writeStringList(cdmaNonRoamingList);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRoamingOverride, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setRoamingOverride(subId, gsmRoamingList, gsmNonRoamingList, cdmaRoamingList, cdmaNonRoamingList);
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
      /**
           * Returns the result and response from RIL for oem request
           *
           * @param oemReq the data is sent to ril.
           * @param oemResp the respose data from RIL.
           * @return negative value request was not handled or get error
           *         0 request was handled succesfully, but no response data
           *         positive value success, data length of response
           */
      @Override public int invokeOemRilRequestRaw(byte[] oemReq, byte[] oemResp) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(oemReq);
          if ((oemResp==null)) {
            _data.writeInt(-1);
          }
          else {
            _data.writeInt(oemResp.length);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_invokeOemRilRequestRaw, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().invokeOemRilRequestRaw(oemReq, oemResp);
          }
          _reply.readException();
          _result = _reply.readInt();
          _reply.readByteArray(oemResp);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Check if any mobile Radios need to be shutdown.
           *
           * @return true is any mobile radio needs to be shutdown
           */
      @Override public boolean needMobileRadioShutdown() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_needMobileRadioShutdown, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().needMobileRadioShutdown();
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
      /**
           * Shutdown Mobile Radios
           */
      @Override public void shutdownMobileRadios() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_shutdownMobileRadios, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().shutdownMobileRadios();
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
           * Set phone radio type and access technology.
           *
           * @param rafs an RadioAccessFamily array to indicate all phone's
           *        new radio access family. The length of RadioAccessFamily
           *        must equ]]al to phone count.
           */
      @Override public void setRadioCapability(android.telephony.RadioAccessFamily[] rafs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedArray(rafs, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRadioCapability, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setRadioCapability(rafs);
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
           * Get phone radio type and access technology.
           *
           * @param phoneId which phone you want to get
           * @param callingPackage the name of the package making the call
           * @return phone radio type and access technology
           */
      @Override public int getRadioAccessFamily(int phoneId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(phoneId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRadioAccessFamily, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRadioAccessFamily(phoneId, callingPackage);
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
           * Enables or disables video calling.
           *
           * @param enable Whether to enable video calling.
           */
      @Override public void enableVideoCalling(boolean enable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableVideoCalling, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enableVideoCalling(enable);
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
           * Whether video calling has been enabled by the user.
           *
           * @param callingPackage The package making the call.
           * @return {@code true} if the user has enabled video calling, {@code false} otherwise.
           */
      @Override public boolean isVideoCallingEnabled(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isVideoCallingEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isVideoCallingEnabled(callingPackage);
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
      /**
           * Whether the DTMF tone length can be changed.
           *
           * @param subId The subscription to use.
           * @param callingPackage The package making the call.
           * @return {@code true} if the DTMF tone length can be changed.
           */
      @Override public boolean canChangeDtmfToneLength(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_canChangeDtmfToneLength, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().canChangeDtmfToneLength(subId, callingPackage);
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
      /**
           * Whether the device is a world phone.
           *
           * @param callingPackage The package making the call.
           * @return {@code true} if the devices is a world phone.
           */
      @Override public boolean isWorldPhone(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isWorldPhone, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isWorldPhone(subId, callingPackage);
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
      /**
           * Whether the phone supports TTY mode.
           *
           * @return {@code true} if the device supports TTY mode.
           */
      @Override public boolean isTtyModeSupported() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isTtyModeSupported, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isTtyModeSupported();
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
      @Override public boolean isRttSupported(int subscriptionId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subscriptionId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isRttSupported, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isRttSupported(subscriptionId);
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
      /**
           * Whether the phone supports hearing aid compatibility.
           *
           * @return {@code true} if the device supports hearing aid compatibility.
           */
      @Override public boolean isHearingAidCompatibilitySupported() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isHearingAidCompatibilitySupported, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isHearingAidCompatibilitySupported();
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
      /**
           * Get IMS Registration Status on a particular subid.
           *
           * @param subId user preferred subId.
           *
           * @return {@code true} if the IMS status is registered.
           */
      @Override public boolean isImsRegistered(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isImsRegistered, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isImsRegistered(subId);
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
      /**
           * Returns the Status of Wi-Fi Calling for the subscription id specified.
           */
      @Override public boolean isWifiCallingAvailable(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isWifiCallingAvailable, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isWifiCallingAvailable(subId);
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
      /**
           * Returns the Status of VT (video telephony) for the subscription ID specified.
           */
      @Override public boolean isVideoTelephonyAvailable(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isVideoTelephonyAvailable, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isVideoTelephonyAvailable(subId);
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
      /**
          * Returns the MMTEL IMS registration technology for the subsciption ID specified.
          */
      @Override public int getImsRegTechnologyForMmTel(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getImsRegTechnologyForMmTel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getImsRegTechnologyForMmTel(subId);
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
            * Returns the unique device ID of phone, for example, the IMEI for
            * GSM and the MEID for CDMA phones. Return null if device ID is not available.
            *
            * @param callingPackage The package making the call.
            * <p>Requires Permission:
            *   {@link android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE}
            */
      @Override public java.lang.String getDeviceId(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDeviceId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDeviceId(callingPackage);
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
      /**
           * Returns the IMEI for the given slot.
           *
           * @param slotIndex - device slot.
           * @param callingPackage The package making the call.
           * <p>Requires Permission:
           *   {@link android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE}
           */
      @Override public java.lang.String getImeiForSlot(int slotIndex, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotIndex);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getImeiForSlot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getImeiForSlot(slotIndex, callingPackage);
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
      /**
           * Returns the Type Allocation Code from the IMEI for the given slot.
           *
           * @param slotIndex - Which slot to retrieve the Type Allocation Code from.
           */
      @Override public java.lang.String getTypeAllocationCodeForSlot(int slotIndex) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotIndex);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTypeAllocationCodeForSlot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTypeAllocationCodeForSlot(slotIndex);
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
      /**
           * Returns the MEID for the given slot.
           *
           * @param slotIndex - device slot.
           * @param callingPackage The package making the call.
           * <p>Requires Permission:
           *   {@link android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE}
           */
      @Override public java.lang.String getMeidForSlot(int slotIndex, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotIndex);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMeidForSlot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMeidForSlot(slotIndex, callingPackage);
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
      /**
           * Returns the Manufacturer Code from the MEID for the given slot.
           *
           * @param slotIndex - Which slot to retrieve the Manufacturer Code from.
           */
      @Override public java.lang.String getManufacturerCodeForSlot(int slotIndex) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotIndex);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getManufacturerCodeForSlot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getManufacturerCodeForSlot(slotIndex);
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
      /**
           * Returns the device software version.
           *
           * @param slotIndex - device slot.
           * @param callingPackage The package making the call.
           * <p>Requires Permission:
           *   {@link android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE}
           */
      @Override public java.lang.String getDeviceSoftwareVersionForSlot(int slotIndex, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotIndex);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDeviceSoftwareVersionForSlot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDeviceSoftwareVersionForSlot(slotIndex, callingPackage);
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
      /**
           * Returns the subscription ID associated with the specified PhoneAccount.
           */
      @Override public int getSubIdForPhoneAccount(android.telecom.PhoneAccount phoneAccount) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((phoneAccount!=null)) {
            _data.writeInt(1);
            phoneAccount.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSubIdForPhoneAccount, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSubIdForPhoneAccount(phoneAccount);
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
           * Returns the PhoneAccountHandle associated with a subscription ID.
           */
      @Override public android.telecom.PhoneAccountHandle getPhoneAccountHandleForSubscriptionId(int subscriptionId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telecom.PhoneAccountHandle _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subscriptionId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPhoneAccountHandleForSubscriptionId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPhoneAccountHandleForSubscriptionId(subscriptionId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telecom.PhoneAccountHandle.CREATOR.createFromParcel(_reply);
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
      @Override public void factoryReset(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_factoryReset, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().factoryReset(subId);
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
           * Returns users's current locale based on the SIM.
           *
           * The returned string will be a well formed BCP-47 language tag, or {@code null}
           * if no locale could be derived.
           */
      @Override public java.lang.String getSimLocaleForSubscriber(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSimLocaleForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSimLocaleForSubscriber(subId);
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
      /**
           * Requests the modem activity info asynchronously.
           * The implementor is expected to reply with the
           * {@link android.telephony.ModemActivityInfo} object placed into the Bundle with the key
           * {@link android.telephony.TelephonyManager#MODEM_ACTIVITY_RESULT_KEY}.
           * The result code is ignored.
           */
      @Override public void requestModemActivityInfo(android.os.ResultReceiver result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((result!=null)) {
            _data.writeInt(1);
            result.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestModemActivityInfo, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestModemActivityInfo(result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Get the service state on specified subscription
           * @param subId Subscription id
           * @param callingPackage The package making the call
           * @return Service state on specified subscription.
           */
      @Override public android.telephony.ServiceState getServiceStateForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.ServiceState _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getServiceStateForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getServiceStateForSubscriber(subId, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telephony.ServiceState.CREATOR.createFromParcel(_reply);
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
           * Returns the URI for the per-account voicemail ringtone set in Phone settings.
           *
           * @param accountHandle The handle for the {@link PhoneAccount} for which to retrieve the
           * voicemail ringtone.
           * @return The URI for the ringtone to play when receiving a voicemail from a specific
           * PhoneAccount.
           */
      @Override public android.net.Uri getVoicemailRingtoneUri(android.telecom.PhoneAccountHandle accountHandle) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.Uri _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((accountHandle!=null)) {
            _data.writeInt(1);
            accountHandle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getVoicemailRingtoneUri, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getVoicemailRingtoneUri(accountHandle);
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
      /**
           * Sets the per-account voicemail ringtone.
           *
           * <p>Requires that the calling app is the default dialer, or has carrier privileges, or
           * has permission {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}.
           *
           * @param phoneAccountHandle The handle for the {@link PhoneAccount} for which to set the
           * voicemail ringtone.
           * @param uri The URI for the ringtone to play when receiving a voicemail from a specific
           * PhoneAccount.
           */
      @Override public void setVoicemailRingtoneUri(java.lang.String callingPackage, android.telecom.PhoneAccountHandle phoneAccountHandle, android.net.Uri uri) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          if ((phoneAccountHandle!=null)) {
            _data.writeInt(1);
            phoneAccountHandle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((uri!=null)) {
            _data.writeInt(1);
            uri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVoicemailRingtoneUri, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVoicemailRingtoneUri(callingPackage, phoneAccountHandle, uri);
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
           * Returns whether vibration is set for voicemail notification in Phone settings.
           *
           * @param accountHandle The handle for the {@link PhoneAccount} for which to retrieve the
           * voicemail vibration setting.
           * @return {@code true} if the vibration is set for this PhoneAccount, {@code false} otherwise.
           */
      @Override public boolean isVoicemailVibrationEnabled(android.telecom.PhoneAccountHandle accountHandle) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((accountHandle!=null)) {
            _data.writeInt(1);
            accountHandle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_isVoicemailVibrationEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isVoicemailVibrationEnabled(accountHandle);
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
      /**
           * Sets the per-account preference whether vibration is enabled for voicemail notifications.
           *
           * <p>Requires that the calling app is the default dialer, or has carrier privileges, or
           * has permission {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}.
           *
           * @param phoneAccountHandle The handle for the {@link PhoneAccount} for which to set the
           * voicemail vibration setting.
           * @param enabled Whether to enable or disable vibration for voicemail notifications from a
           * specific PhoneAccount.
           */
      @Override public void setVoicemailVibrationEnabled(java.lang.String callingPackage, android.telecom.PhoneAccountHandle phoneAccountHandle, boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          if ((phoneAccountHandle!=null)) {
            _data.writeInt(1);
            phoneAccountHandle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVoicemailVibrationEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVoicemailVibrationEnabled(callingPackage, phoneAccountHandle, enabled);
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
           * Returns a list of packages that have carrier privileges for the specific phone.
           */
      @Override public java.util.List<java.lang.String> getPackagesWithCarrierPrivileges(int phoneId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<java.lang.String> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(phoneId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPackagesWithCarrierPrivileges, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPackagesWithCarrierPrivileges(phoneId);
          }
          _reply.readException();
          _result = _reply.createStringArrayList();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
            * Returns a list of packages that have carrier privileges.
            */
      @Override public java.util.List<java.lang.String> getPackagesWithCarrierPrivilegesForAllPhones() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<java.lang.String> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPackagesWithCarrierPrivilegesForAllPhones, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPackagesWithCarrierPrivilegesForAllPhones();
          }
          _reply.readException();
          _result = _reply.createStringArrayList();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Return the application ID for the app type.
           *
           * @param subId the subscription ID that this request applies to.
           * @param appType the uicc app type,
           * @return Application ID for specificied app type or null if no uicc or error.
           */
      @Override public java.lang.String getAidForAppType(int subId, int appType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(appType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAidForAppType, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAidForAppType(subId, appType);
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
      /**
          * Return the Electronic Serial Number.
          *
          * Requires that the calling app has READ_PRIVILEGED_PHONE_STATE permission
          *
          * @param subId the subscription ID that this request applies to.
          * @return ESN or null if error.
          * @hide
          */
      @Override public java.lang.String getEsn(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getEsn, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getEsn(subId);
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
      /**
          * Return the Preferred Roaming List Version
          *
          * Requires that the calling app has READ_PRIVILEGED_PHONE_STATE permission
          * @param subId the subscription ID that this request applies to.
          * @return PRLVersion or null if error.
          * @hide
          */
      @Override public java.lang.String getCdmaPrlVersion(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCdmaPrlVersion, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCdmaPrlVersion(subId);
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
      /**
           * Get snapshot of Telephony histograms
           * @return List of Telephony histograms
           * Requires Permission:
           *   {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}
           * Or the calling app has carrier privileges.
           */
      @Override public java.util.List<android.telephony.TelephonyHistogram> getTelephonyHistograms() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.telephony.TelephonyHistogram> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTelephonyHistograms, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTelephonyHistograms();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.telephony.TelephonyHistogram.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Set the allowed carrier list and the excluded carrier list, indicating the priority between
           * the two lists.
           *
           * <p>Requires system privileges. In the future we may add this to carrier APIs.
           *
           * @return {@link #SET_CARRIER_RESTRICTION_SUCCESS} in case of success.
           * {@link #SET_CARRIER_RESTRICTION_NOT_SUPPORTED} if the modem does not support the
           * configuration. {@link #SET_CARRIER_RESTRICTION_ERROR} in all other error cases.
           */
      @Override public int setAllowedCarriers(android.telephony.CarrierRestrictionRules carrierRestrictionRules) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((carrierRestrictionRules!=null)) {
            _data.writeInt(1);
            carrierRestrictionRules.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAllowedCarriers, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setAllowedCarriers(carrierRestrictionRules);
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
           * Get the allowed carrier list and the excluded carrier list indicating the priority between
           * the two lists.
           *
           * <p>Requires system privileges. In the future we may add this to carrier APIs.
           *
           * @return {@link CarrierRestrictionRules}; empty lists mean all carriers are allowed. It
           * returns null in case of error.
           */
      @Override public android.telephony.CarrierRestrictionRules getAllowedCarriers() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.CarrierRestrictionRules _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAllowedCarriers, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAllowedCarriers();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telephony.CarrierRestrictionRules.CREATOR.createFromParcel(_reply);
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
           * Returns carrier id of the given subscription.
           * <p>To recognize carrier as a first class identity, assign each carrier with a canonical
           * integer a.k.a carrier id.
           *
           * @param subId The subscription id
           * @return Carrier id of given subscription id. return {@link #UNKNOWN_CARRIER_ID} if
           * subscription is unavailable or carrier cannot be identified.
           * @throws IllegalStateException if telephony service is unavailable.
           * @hide
           */
      @Override public int getSubscriptionCarrierId(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSubscriptionCarrierId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSubscriptionCarrierId(subId);
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
           * Returns carrier name of the given subscription.
           * <p>Carrier name is a user-facing name of carrier id {@link #getSimCarrierId(int)},
           * usually the brand name of the subsidiary (e.g. T-Mobile). Each carrier could configure
           * multiple {@link #getSimOperatorName() SPN} but should have a single carrier name.
           * Carrier name is not canonical identity, use {@link #getSimCarrierId(int)} instead.
           * <p>Returned carrier name is unlocalized.
           *
           * @return Carrier name of given subscription id. return {@code null} if subscription is
           * unavailable or carrier cannot be identified.
           * @throws IllegalStateException if telephony service is unavailable.
           * @hide
           */
      @Override public java.lang.String getSubscriptionCarrierName(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSubscriptionCarrierName, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSubscriptionCarrierName(subId);
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
      /**
           * Returns fine-grained carrier id of the current subscription.
           *
           * <p>The specific carrier id can be used to further differentiate a carrier by different
           * networks, by prepaid v.s.postpaid or even by 4G v.s.3G plan. Each carrier has a unique
           * carrier id {@link #getSimCarrierId()} but can have multiple precise carrier id. e.g,
           * {@link #getSimCarrierId()} will always return Tracfone (id 2022) for a Tracfone SIM, while
           * {@link #getSimPreciseCarrierId()} can return Tracfone AT&T or Tracfone T-Mobile based on the
           * current underlying network.
           *
           * <p>For carriers without any fine-grained carrier ids, return {@link #getSimCarrierId()}
           *
           * @return Returns fine-grained carrier id of the current subscription.
           * Return {@link #UNKNOWN_CARRIER_ID} if the subscription is unavailable or the carrier cannot
           * be identified.
           * @hide
           */
      @Override public int getSubscriptionSpecificCarrierId(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSubscriptionSpecificCarrierId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSubscriptionSpecificCarrierId(subId);
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
           * Similar like {@link #getSimCarrierIdName()}, returns user-facing name of the
           * specific carrier id {@link #getSimSpecificCarrierId()}
           *
           * <p>The returned name is unlocalized.
           *
           * @return user-facing name of the subscription specific carrier id. Return {@code null} if the
           * subscription is unavailable or the carrier cannot be identified.
           * @hide
           */
      @Override public java.lang.String getSubscriptionSpecificCarrierName(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSubscriptionSpecificCarrierName, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSubscriptionSpecificCarrierName(subId);
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
      /**
           * Returns carrier id based on MCCMNC only. This will return a MNO carrier id used for fallback
           * check when exact carrier id {@link #getSimCarrierId()} configurations are not found
           *
           * @param isSubscriptionMccMnc. If {@true} it means this is a query for subscription mccmnc
           * {@false} otherwise.
           *
           * @return carrier id from passing mccmnc.
           * @hide
           */
      @Override public int getCarrierIdFromMccMnc(int slotIndex, java.lang.String mccmnc, boolean isSubscriptionMccMnc) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotIndex);
          _data.writeString(mccmnc);
          _data.writeInt(((isSubscriptionMccMnc)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCarrierIdFromMccMnc, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCarrierIdFromMccMnc(slotIndex, mccmnc, isSubscriptionMccMnc);
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
           * Action set from carrier signalling broadcast receivers to enable/disable metered apns
           * Permissions android.Manifest.permission.MODIFY_PHONE_STATE is required
           * @param subId the subscription ID that this action applies to.
           * @param enabled control enable or disable metered apns.
           * @hide
           */
      @Override public void carrierActionSetMeteredApnsEnabled(int subId, boolean visible) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(((visible)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_carrierActionSetMeteredApnsEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().carrierActionSetMeteredApnsEnabled(subId, visible);
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
           * Action set from carrier signalling broadcast receivers to enable/disable radio
           * Permissions android.Manifest.permission.MODIFY_PHONE_STATE is required
           * @param subId the subscription ID that this action applies to.
           * @param enabled control enable or disable radio.
           * @hide
           */
      @Override public void carrierActionSetRadioEnabled(int subId, boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_carrierActionSetRadioEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().carrierActionSetRadioEnabled(subId, enabled);
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
           * Action set from carrier signalling broadcast receivers to start/stop reporting default
           * network conditions.
           * Permissions android.Manifest.permission.MODIFY_PHONE_STATE is required
           * @param subId the subscription ID that this action applies to.
           * @param report control start/stop reporting default network events.
           * @hide
           */
      @Override public void carrierActionReportDefaultNetworkStatus(int subId, boolean report) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(((report)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_carrierActionReportDefaultNetworkStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().carrierActionReportDefaultNetworkStatus(subId, report);
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
           * Action set from carrier signalling broadcast receivers to reset all carrier actions.
           * Permissions android.Manifest.permission.MODIFY_PHONE_STATE is required
           * @param subId the subscription ID that this action applies to.
           * @hide
           */
      @Override public void carrierActionResetAll(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_carrierActionResetAll, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().carrierActionResetAll(subId);
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
           * Get aggregated video call data usage since boot.
           * Permissions android.Manifest.permission.READ_NETWORK_USAGE_HISTORY is required.
           *
           * @param perUidStats True if requesting data usage per uid, otherwise overall usage.
           * @return Snapshot of video call data usage
           * @hide
           */
      @Override public android.net.NetworkStats getVtDataUsage(int subId, boolean perUidStats) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.NetworkStats _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(((perUidStats)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getVtDataUsage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getVtDataUsage(subId, perUidStats);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.NetworkStats.CREATOR.createFromParcel(_reply);
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
           * Policy control of data connection. Usually used when data limit is passed.
           * @param enabled True if enabling the data, otherwise disabling.
           * @param subId Subscription index
           * @hide
           */
      @Override public void setPolicyDataEnabled(boolean enabled, int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPolicyDataEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPolicyDataEnabled(enabled, subId);
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
           * Get Client request stats which will contain statistical information
           * on each request made by client.
           * @param callingPackage package making the call.
           * @param subId Subscription index
           * @hide
           */
      @Override public java.util.List<android.telephony.ClientRequestStats> getClientRequestStats(java.lang.String callingPackage, int subid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.telephony.ClientRequestStats> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(subid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getClientRequestStats, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getClientRequestStats(callingPackage, subid);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.telephony.ClientRequestStats.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Set SIM card power state.
           * @param slotIndex SIM slot id
           * @param state  State of SIM (power down, power up, pass through)
           * @hide
           * */
      @Override public void setSimPowerStateForSlot(int slotIndex, int state) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotIndex);
          _data.writeInt(state);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSimPowerStateForSlot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSimPowerStateForSlot(slotIndex, state);
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
           * Returns a list of Forbidden PLMNs from the specified SIM App
           * Returns null if the query fails.
           *
           * <p>Requires that the calling app has READ_PRIVILEGED_PHONE_STATE or READ_PHONE_STATE
           *
           * @param subId subscription ID used for authentication
           * @param appType the icc application type, like {@link #APPTYPE_USIM}
           */
      @Override public java.lang.String[] getForbiddenPlmns(int subId, int appType, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(appType);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getForbiddenPlmns, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getForbiddenPlmns(subId, appType, callingPackage);
          }
          _reply.readException();
          _result = _reply.createStringArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Check if phone is in emergency callback mode
           * @return true if phone is in emergency callback mode
           * @param subId the subscription ID that this action applies to.
           * @hide
           */
      @Override public boolean getEmergencyCallbackMode(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getEmergencyCallbackMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getEmergencyCallbackMode(subId);
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
      /**
           * Get the most recently available signal strength information.
           *
           * Get the most recent SignalStrength information reported by the modem. Due
           * to power saving this information may not always be current.
           * @param subId Subscription index
           * @return the most recent cached signal strength info from the modem
           * @hide
           */
      @Override public android.telephony.SignalStrength getSignalStrength(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.SignalStrength _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSignalStrength, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSignalStrength(subId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telephony.SignalStrength.CREATOR.createFromParcel(_reply);
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
           * Get the card ID of the default eUICC card. If there is no eUICC, returns
           * {@link #INVALID_CARD_ID}.
           *
           * @param subId subscription ID used for authentication
           * @param callingPackage package making the call
           * @return card ID of the default eUICC card.
           */
      @Override public int getCardIdForDefaultEuicc(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCardIdForDefaultEuicc, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCardIdForDefaultEuicc(subId, callingPackage);
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
           * Gets information about currently inserted UICCs and eUICCs.
           * <p>
           * Requires that the calling app has carrier privileges (see {@link #hasCarrierPrivileges}).
           * <p>
           * If the caller has carrier priviliges on any active subscription, then they have permission to
           * get simple information like the card ID ({@link UiccCardInfo#getCardId()}), whether the card
           * is an eUICC ({@link UiccCardInfo#isEuicc()}), and the slot index where the card is inserted
           * ({@link UiccCardInfo#getSlotIndex()}).
           * <p>
           * To get private information such as the EID ({@link UiccCardInfo#getEid()}) or ICCID
           * ({@link UiccCardInfo#getIccId()}), the caller must have carrier priviliges on that specific
           * UICC or eUICC card.
           * <p>
           * See {@link UiccCardInfo} for more details on the kind of information available.
           *
           * @param callingPackage package making the call, used to evaluate carrier privileges
           * @return a list of UiccCardInfo objects, representing information on the currently inserted
           * UICCs and eUICCs. Each UiccCardInfo in the list will have private information filtered out if
           * the caller does not have adequate permissions for that card.
           */
      @Override public java.util.List<android.telephony.UiccCardInfo> getUiccCardsInfo(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.telephony.UiccCardInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getUiccCardsInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getUiccCardsInfo(callingPackage);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.telephony.UiccCardInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Get slot info for all the UICC slots.
           * @return UiccSlotInfo array.
           * @hide
           */
      @Override public android.telephony.UiccSlotInfo[] getUiccSlotsInfo() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.UiccSlotInfo[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getUiccSlotsInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getUiccSlotsInfo();
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.telephony.UiccSlotInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Map logicalSlot to physicalSlot, and activate the physicalSlot if it is inactive.
           * @param physicalSlots Index i in the array representing physical slot for phone i. The array
           *        size should be same as getPhoneCount().
           * @return boolean Return true if the switch succeeds, false if the switch fails.
           */
      @Override public boolean switchSlots(int[] physicalSlots) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeIntArray(physicalSlots);
          boolean _status = mRemote.transact(Stub.TRANSACTION_switchSlots, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().switchSlots(physicalSlots);
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
      /**
           * Sets radio indication update mode. This can be used to control the behavior of indication
           * update from modem to Android frameworks. For example, by default several indication updates
           * are turned off when screen is off, but in some special cases (e.g. carkit is connected but
           * screen is off) we want to turn on those indications even when the screen is off.
           */
      @Override public void setRadioIndicationUpdateMode(int subId, int filters, int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(filters);
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRadioIndicationUpdateMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setRadioIndicationUpdateMode(subId, filters, mode);
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
           * Returns whether mobile data roaming is enabled on the subscription with id {@code subId}.
           *
           * @param subId the subscription id
           * @return {@code true} if the data roaming is enabled on this subscription.
           */
      @Override public boolean isDataRoamingEnabled(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isDataRoamingEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isDataRoamingEnabled(subId);
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
      /**
           * Enables/Disables the data roaming on the subscription with id {@code subId}.
           *
           * @param subId the subscription id
           * @param isEnabled {@code true} to enable mobile data roaming, otherwise disable it.
           */
      @Override public void setDataRoamingEnabled(int subId, boolean isEnabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(((isEnabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDataRoamingEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDataRoamingEnabled(subId, isEnabled);
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
           * Gets the roaming mode for the CDMA phone with the subscription id {@code subId}.
           *
           * @param the subscription id.
           * @return the roaming mode for CDMA phone.
           */
      @Override public int getCdmaRoamingMode(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCdmaRoamingMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCdmaRoamingMode(subId);
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
           * Sets the roaming mode on the CDMA phone with the subscription {@code subId} to the given
           * roaming mode {@code mode}.
           *
           * @param subId the subscription id.
           * @param mode the roaming mode should be set.
           * @return {@code true} if successed.
           */
      @Override public boolean setCdmaRoamingMode(int subId, int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setCdmaRoamingMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setCdmaRoamingMode(subId, mode);
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
      /**
           * Sets the subscription mode for CDMA phone with the subscription {@code subId} to the given
           * subscription mode {@code mode}.
           *
           * @param subId the subscription id.
           * @param mode the subscription mode should be set.
           * @return {@code true} if successed.
           */
      @Override public boolean setCdmaSubscriptionMode(int subId, int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setCdmaSubscriptionMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setCdmaSubscriptionMode(subId, mode);
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
      /**
           * A test API to override carrier information including mccmnc, imsi, iccid, gid1, gid2,
           * plmn and spn. This would be handy for, eg, forcing a particular carrier id, carrier's config
           * (also any country or carrier overlays) to be loaded when using a test SIM with a call box.
           */
      @Override public void setCarrierTestOverride(int subId, java.lang.String mccmnc, java.lang.String imsi, java.lang.String iccid, java.lang.String gid1, java.lang.String gid2, java.lang.String plmn, java.lang.String spn, java.lang.String carrierPrivilegeRules, java.lang.String apn) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(mccmnc);
          _data.writeString(imsi);
          _data.writeString(iccid);
          _data.writeString(gid1);
          _data.writeString(gid2);
          _data.writeString(plmn);
          _data.writeString(spn);
          _data.writeString(carrierPrivilegeRules);
          _data.writeString(apn);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setCarrierTestOverride, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setCarrierTestOverride(subId, mccmnc, imsi, iccid, gid1, gid2, plmn, spn, carrierPrivilegeRules, apn);
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
           * A test API to return installed carrier id list version.
           */
      @Override public int getCarrierIdListVersion(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCarrierIdListVersion, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCarrierIdListVersion(subId);
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
           * A test API to reload the UICC profile.
           * @hide
           */
      @Override public void refreshUiccProfile(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_refreshUiccProfile, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().refreshUiccProfile(subId);
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
           * How many modems can have simultaneous data connections.
           * @hide
           */
      @Override public int getNumberOfModemsWithSimultaneousDataConnections(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNumberOfModemsWithSimultaneousDataConnections, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNumberOfModemsWithSimultaneousDataConnections(subId, callingPackage);
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
           * Return the network selection mode on the subscription with id {@code subId}.
           */
      @Override public int getNetworkSelectionMode(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNetworkSelectionMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNetworkSelectionMode(subId);
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
           * Return true if the device is in emergency sms mode, false otherwise.
           */
      @Override public boolean isInEmergencySmsMode() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isInEmergencySmsMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isInEmergencySmsMode();
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
      /**
           * Get a list of SMS apps on a user.
           */
      @Override public java.lang.String[] getSmsApps(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSmsApps, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSmsApps(userId);
          }
          _reply.readException();
          _result = _reply.createStringArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Get the default SMS app on a given user.
           */
      @Override public java.lang.String getDefaultSmsApp(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDefaultSmsApp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDefaultSmsApp(userId);
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
      /**
           * Set the default SMS app to a given package on a given user.
           */
      @Override public void setDefaultSmsApp(int userId, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDefaultSmsApp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDefaultSmsApp(userId, packageName);
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
           * Return the modem radio power state for slot index.
           *
           */
      @Override public int getRadioPowerState(int slotIndex, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotIndex);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRadioPowerState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRadioPowerState(slotIndex, callingPackage);
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
      // IMS specific AIDL commands, see ImsMmTelManager.java
      /**
           * Adds an IMS registration status callback for the subscription id specified.
           */
      @Override public void registerImsRegistrationCallback(int subId, android.telephony.ims.aidl.IImsRegistrationCallback c) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeStrongBinder((((c!=null))?(c.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerImsRegistrationCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerImsRegistrationCallback(subId, c);
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
            * Removes an existing IMS registration status callback for the subscription specified.
            */
      @Override public void unregisterImsRegistrationCallback(int subId, android.telephony.ims.aidl.IImsRegistrationCallback c) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeStrongBinder((((c!=null))?(c.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterImsRegistrationCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterImsRegistrationCallback(subId, c);
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
           * Adds an IMS MmTel capabilities callback for the subscription specified.
           */
      @Override public void registerMmTelCapabilityCallback(int subId, android.telephony.ims.aidl.IImsCapabilityCallback c) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeStrongBinder((((c!=null))?(c.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerMmTelCapabilityCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerMmTelCapabilityCallback(subId, c);
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
           * Removes an existing IMS MmTel capabilities callback for the subscription specified.
           */
      @Override public void unregisterMmTelCapabilityCallback(int subId, android.telephony.ims.aidl.IImsCapabilityCallback c) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeStrongBinder((((c!=null))?(c.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterMmTelCapabilityCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterMmTelCapabilityCallback(subId, c);
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
           * return true if the IMS MmTel capability for the given registration tech is capable.
           */
      @Override public boolean isCapable(int subId, int capability, int regTech) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(capability);
          _data.writeInt(regTech);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isCapable, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isCapable(subId, capability, regTech);
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
      /**
           * return true if the IMS MmTel capability for the given registration tech is available.
           */
      @Override public boolean isAvailable(int subId, int capability, int regTech) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(capability);
          _data.writeInt(regTech);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isAvailable, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isAvailable(subId, capability, regTech);
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
      /**
           * Returns true if the user's setting for 4G LTE is enabled, for the subscription specified.
           */
      @Override public boolean isAdvancedCallingSettingEnabled(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isAdvancedCallingSettingEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isAdvancedCallingSettingEnabled(subId);
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
      /**
           * Modify the user's setting for whether or not 4G LTE is enabled.
           */
      @Override public void setAdvancedCallingSettingEnabled(int subId, boolean isEnabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(((isEnabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAdvancedCallingSettingEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAdvancedCallingSettingEnabled(subId, isEnabled);
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
           * return true if the user's setting for VT is enabled for the subscription.
           */
      @Override public boolean isVtSettingEnabled(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isVtSettingEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isVtSettingEnabled(subId);
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
      /**
           * Modify the user's setting for whether or not VT is available for the subscrption specified.
           */
      @Override public void setVtSettingEnabled(int subId, boolean isEnabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(((isEnabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVtSettingEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVtSettingEnabled(subId, isEnabled);
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
           * return true if the user's setting for whether or not Voice over WiFi is currently enabled.
           */
      @Override public boolean isVoWiFiSettingEnabled(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isVoWiFiSettingEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isVoWiFiSettingEnabled(subId);
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
      /**
           * sets the user's setting for Voice over WiFi enabled state.
           */
      @Override public void setVoWiFiSettingEnabled(int subId, boolean isEnabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(((isEnabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVoWiFiSettingEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVoWiFiSettingEnabled(subId, isEnabled);
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
           * return true if the user's setting for Voice over WiFi while roaming is enabled.
           */
      @Override public boolean isVoWiFiRoamingSettingEnabled(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isVoWiFiRoamingSettingEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isVoWiFiRoamingSettingEnabled(subId);
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
      /**
           * Sets the user's preference for whether or not Voice over WiFi is enabled for the current
           * subscription while roaming.
           */
      @Override public void setVoWiFiRoamingSettingEnabled(int subId, boolean isEnabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(((isEnabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVoWiFiRoamingSettingEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVoWiFiRoamingSettingEnabled(subId, isEnabled);
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
           * Set the Voice over WiFi enabled state, but do not persist the setting.
           */
      @Override public void setVoWiFiNonPersistent(int subId, boolean isCapable, int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(((isCapable)?(1):(0)));
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVoWiFiNonPersistent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVoWiFiNonPersistent(subId, isCapable, mode);
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
           * return the Voice over WiFi mode preference set by the user for the subscription specified.
           */
      @Override public int getVoWiFiModeSetting(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getVoWiFiModeSetting, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getVoWiFiModeSetting(subId);
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
           * sets the user's preference for the Voice over WiFi mode for the subscription specified.
           */
      @Override public void setVoWiFiModeSetting(int subId, int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVoWiFiModeSetting, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVoWiFiModeSetting(subId, mode);
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
           * return the Voice over WiFi mode preference set by the user for the subscription specified
           * while roaming.
           */
      @Override public int getVoWiFiRoamingModeSetting(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getVoWiFiRoamingModeSetting, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getVoWiFiRoamingModeSetting(subId);
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
           * sets the user's preference for the Voice over WiFi mode for the subscription specified
           * while roaming.
           */
      @Override public void setVoWiFiRoamingModeSetting(int subId, int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVoWiFiRoamingModeSetting, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVoWiFiRoamingModeSetting(subId, mode);
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
           * Modify the user's setting for whether or not RTT is enabled for the subscrption specified.
           */
      @Override public void setRttCapabilitySetting(int subId, boolean isEnabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(((isEnabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRttCapabilitySetting, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setRttCapabilitySetting(subId, isEnabled);
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
           * return true if TTY over VoLTE is enabled for the subscription specified.
           */
      @Override public boolean isTtyOverVolteEnabled(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isTtyOverVolteEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isTtyOverVolteEnabled(subId);
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
      /**
           * Return the emergency number list from all the active subscriptions.
           */
      @Override public java.util.Map getEmergencyNumberList(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.Map _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getEmergencyNumberList, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getEmergencyNumberList(callingPackage);
          }
          _reply.readException();
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _result = _reply.readHashMap(cl);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Identify if the number is emergency number, based on all the active subscriptions.
           */
      @Override public boolean isEmergencyNumber(java.lang.String number, boolean exactMatch) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(number);
          _data.writeInt(((exactMatch)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_isEmergencyNumber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isEmergencyNumber(number, exactMatch);
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
      /**
           * Return a list of certs in hex string from loaded carrier privileges access rules.
           */
      @Override public java.util.List<java.lang.String> getCertsFromCarrierPrivilegeAccessRules(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<java.lang.String> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCertsFromCarrierPrivilegeAccessRules, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCertsFromCarrierPrivilegeAccessRules(subId);
          }
          _reply.readException();
          _result = _reply.createStringArrayList();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Register an IMS provisioning change callback with Telephony.
           */
      @Override public void registerImsProvisioningChangedCallback(int subId, android.telephony.ims.aidl.IImsConfigCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerImsProvisioningChangedCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerImsProvisioningChangedCallback(subId, callback);
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
           * unregister an existing IMS provisioning change callback.
           */
      @Override public void unregisterImsProvisioningChangedCallback(int subId, android.telephony.ims.aidl.IImsConfigCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterImsProvisioningChangedCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterImsProvisioningChangedCallback(subId, callback);
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
           * Set the provisioning status for the IMS MmTel capability using the specified subscription.
           */
      @Override public void setImsProvisioningStatusForCapability(int subId, int capability, int tech, boolean isProvisioned) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(capability);
          _data.writeInt(tech);
          _data.writeInt(((isProvisioned)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setImsProvisioningStatusForCapability, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setImsProvisioningStatusForCapability(subId, capability, tech, isProvisioned);
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
           * Get the provisioning status for the IMS MmTel capability specified.
           */
      @Override public boolean getImsProvisioningStatusForCapability(int subId, int capability, int tech) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(capability);
          _data.writeInt(tech);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getImsProvisioningStatusForCapability, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getImsProvisioningStatusForCapability(subId, capability, tech);
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
      /** Is the capability and tech flagged as provisioned in the cache */
      @Override public boolean isMmTelCapabilityProvisionedInCache(int subId, int capability, int tech) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(capability);
          _data.writeInt(tech);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isMmTelCapabilityProvisionedInCache, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isMmTelCapabilityProvisionedInCache(subId, capability, tech);
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
      /** Set the provisioning for the capability and tech in the cache */
      @Override public void cacheMmTelCapabilityProvisioning(int subId, int capability, int tech, boolean isProvisioned) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(capability);
          _data.writeInt(tech);
          _data.writeInt(((isProvisioned)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_cacheMmTelCapabilityProvisioning, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cacheMmTelCapabilityProvisioning(subId, capability, tech, isProvisioned);
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
           * Return an integer containing the provisioning value for the specified provisioning key.
           */
      @Override public int getImsProvisioningInt(int subId, int key) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(key);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getImsProvisioningInt, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getImsProvisioningInt(subId, key);
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
           * return a String containing the provisioning value for the provisioning key specified.
           */
      @Override public java.lang.String getImsProvisioningString(int subId, int key) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(key);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getImsProvisioningString, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getImsProvisioningString(subId, key);
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
      /**
           * Set the integer provisioning value for the provisioning key specified.
           */
      @Override public int setImsProvisioningInt(int subId, int key, int value) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(key);
          _data.writeInt(value);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setImsProvisioningInt, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setImsProvisioningInt(subId, key, value);
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
           * Set the String provisioning value for the provisioning key specified.
           */
      @Override public int setImsProvisioningString(int subId, int key, java.lang.String value) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(key);
          _data.writeString(value);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setImsProvisioningString, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setImsProvisioningString(subId, key, value);
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
           * Update Emergency Number List for Test Mode.
           */
      @Override public void updateEmergencyNumberListTestMode(int action, android.telephony.emergency.EmergencyNumber num) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(action);
          if ((num!=null)) {
            _data.writeInt(1);
            num.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateEmergencyNumberListTestMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateEmergencyNumberListTestMode(action, num);
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
           * Get the full emergency number list for Test Mode.
           */
      @Override public java.util.List<java.lang.String> getEmergencyNumberListTestMode() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<java.lang.String> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getEmergencyNumberListTestMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getEmergencyNumberListTestMode();
          }
          _reply.readException();
          _result = _reply.createStringArrayList();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Enable or disable a logical modem stack associated with the slotIndex.
           */
      @Override public boolean enableModemForSlot(int slotIndex, boolean enable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotIndex);
          _data.writeInt(((enable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableModemForSlot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().enableModemForSlot(slotIndex, enable);
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
      /**
           * Indicate if the enablement of multi SIM functionality is restricted.
           * @hide
           */
      @Override public void setMultiSimCarrierRestriction(boolean isMultiSimCarrierRestricted) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((isMultiSimCarrierRestricted)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setMultiSimCarrierRestriction, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setMultiSimCarrierRestriction(isMultiSimCarrierRestricted);
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
           * Returns if the usage of multiple SIM cards at the same time is supported.
           *
           * @param callingPackage The package making the call.
           * @return {@link #MULTISIM_ALLOWED} if the device supports multiple SIMs.
           * {@link #MULTISIM_NOT_SUPPORTED_BY_HARDWARE} if the device does not support multiple SIMs.
           * {@link #MULTISIM_NOT_SUPPORTED_BY_CARRIER} in the device supports multiple SIMs, but the
           * functionality is restricted by the carrier.
           */
      @Override public int isMultiSimSupported(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isMultiSimSupported, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isMultiSimSupported(callingPackage);
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
           * Switch configs to enable multi-sim or switch back to single-sim
           * @hide
           */
      @Override public void switchMultiSimConfig(int numOfSims) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(numOfSims);
          boolean _status = mRemote.transact(Stub.TRANSACTION_switchMultiSimConfig, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().switchMultiSimConfig(numOfSims);
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
           * Get if altering modems configurations will trigger reboot.
           * @hide
           */
      @Override public boolean doesSwitchMultiSimConfigTriggerReboot(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_doesSwitchMultiSimConfigTriggerReboot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().doesSwitchMultiSimConfigTriggerReboot(subId, callingPackage);
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
      /**
           * Get the mapping from logical slots to physical slots.
           */
      @Override public int[] getSlotsMapping() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSlotsMapping, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSlotsMapping();
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
      /**
           * Get the IRadio HAL Version encoded as 100 * MAJOR_VERSION + MINOR_VERSION or -1 if unknown
           */
      @Override public int getRadioHalVersion() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRadioHalVersion, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRadioHalVersion();
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
      @Override public boolean isModemEnabledForSlot(int slotIndex, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotIndex);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isModemEnabledForSlot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isModemEnabledForSlot(slotIndex, callingPackage);
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
      @Override public boolean isDataEnabledForApn(int apnType, int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(apnType);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isDataEnabledForApn, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isDataEnabledForApn(apnType, subId, callingPackage);
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
      @Override public boolean isApnMetered(int apnType, int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(apnType);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isApnMetered, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isApnMetered(apnType, subId);
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
      /**
           * Enqueue a pending sms Consumer, which will answer with the user specified selection for an
           * outgoing SmsManager operation.
           */
      @Override public void enqueueSmsPickResult(java.lang.String callingPackage, com.android.internal.telephony.IIntegerConsumer subIdResult) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeStrongBinder((((subIdResult!=null))?(subIdResult.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_enqueueSmsPickResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enqueueSmsPickResult(callingPackage, subIdResult);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Returns the MMS user agent.
           */
      @Override public java.lang.String getMmsUserAgent(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMmsUserAgent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMmsUserAgent(subId);
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
      /**
           * Returns the MMS user agent profile URL.
           */
      @Override public java.lang.String getMmsUAProfUrl(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMmsUAProfUrl, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMmsUAProfUrl(subId);
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
      /**
           * Set allowing mobile data during voice call.
           */
      @Override public boolean setDataAllowedDuringVoiceCall(int subId, boolean allow) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(((allow)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDataAllowedDuringVoiceCall, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setDataAllowedDuringVoiceCall(subId, allow);
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
      /**
           * Check whether data is allowed during voice call. Note this is for dual sim device that
           * data might be disabled on non-default data subscription but explicitly turned on by settings.
           */
      @Override public boolean isDataAllowedInVoiceCall(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isDataAllowedInVoiceCall, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isDataAllowedInVoiceCall(subId);
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
      public static com.android.internal.telephony.ITelephony sDefaultImpl;
    }
    static final int TRANSACTION_dial = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_call = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_isRadioOn = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_isRadioOnForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_supplyPin = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_supplyPinForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_supplyPuk = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_supplyPukForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_supplyPinReportResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_supplyPinReportResultForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_supplyPukReportResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_supplyPukReportResultForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_handlePinMmi = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_handleUssdRequest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_handlePinMmiForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_toggleRadioOnOff = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_toggleRadioOnOffForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_setRadio = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_setRadioForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_setRadioPower = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_updateServiceLocation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_updateServiceLocationForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_enableLocationUpdates = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_enableLocationUpdatesForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_disableLocationUpdates = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_disableLocationUpdatesForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_enableDataConnectivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_disableDataConnectivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_isDataConnectivityPossible = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_getCellLocation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_getNetworkCountryIsoForPhone = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_getNeighboringCellInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_getCallState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_getCallStateForSlot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_getDataActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_getDataActivityForSubId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_getDataState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    static final int TRANSACTION_getDataStateForSubId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 37);
    static final int TRANSACTION_getActivePhoneType = (android.os.IBinder.FIRST_CALL_TRANSACTION + 38);
    static final int TRANSACTION_getActivePhoneTypeForSlot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 39);
    static final int TRANSACTION_getCdmaEriIconIndex = (android.os.IBinder.FIRST_CALL_TRANSACTION + 40);
    static final int TRANSACTION_getCdmaEriIconIndexForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 41);
    static final int TRANSACTION_getCdmaEriIconMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 42);
    static final int TRANSACTION_getCdmaEriIconModeForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 43);
    static final int TRANSACTION_getCdmaEriText = (android.os.IBinder.FIRST_CALL_TRANSACTION + 44);
    static final int TRANSACTION_getCdmaEriTextForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 45);
    static final int TRANSACTION_needsOtaServiceProvisioning = (android.os.IBinder.FIRST_CALL_TRANSACTION + 46);
    static final int TRANSACTION_setVoiceMailNumber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 47);
    static final int TRANSACTION_setVoiceActivationState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 48);
    static final int TRANSACTION_setDataActivationState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 49);
    static final int TRANSACTION_getVoiceActivationState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 50);
    static final int TRANSACTION_getDataActivationState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 51);
    static final int TRANSACTION_getVoiceMessageCountForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 52);
    static final int TRANSACTION_isConcurrentVoiceAndDataAllowed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 53);
    static final int TRANSACTION_getVisualVoicemailSettings = (android.os.IBinder.FIRST_CALL_TRANSACTION + 54);
    static final int TRANSACTION_getVisualVoicemailPackageName = (android.os.IBinder.FIRST_CALL_TRANSACTION + 55);
    static final int TRANSACTION_enableVisualVoicemailSmsFilter = (android.os.IBinder.FIRST_CALL_TRANSACTION + 56);
    static final int TRANSACTION_disableVisualVoicemailSmsFilter = (android.os.IBinder.FIRST_CALL_TRANSACTION + 57);
    static final int TRANSACTION_getVisualVoicemailSmsFilterSettings = (android.os.IBinder.FIRST_CALL_TRANSACTION + 58);
    static final int TRANSACTION_getActiveVisualVoicemailSmsFilterSettings = (android.os.IBinder.FIRST_CALL_TRANSACTION + 59);
    static final int TRANSACTION_sendVisualVoicemailSmsForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 60);
    static final int TRANSACTION_sendDialerSpecialCode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 61);
    static final int TRANSACTION_getNetworkTypeForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 62);
    static final int TRANSACTION_getDataNetworkType = (android.os.IBinder.FIRST_CALL_TRANSACTION + 63);
    static final int TRANSACTION_getDataNetworkTypeForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 64);
    static final int TRANSACTION_getVoiceNetworkTypeForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 65);
    static final int TRANSACTION_hasIccCard = (android.os.IBinder.FIRST_CALL_TRANSACTION + 66);
    static final int TRANSACTION_hasIccCardUsingSlotIndex = (android.os.IBinder.FIRST_CALL_TRANSACTION + 67);
    static final int TRANSACTION_getLteOnCdmaMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 68);
    static final int TRANSACTION_getLteOnCdmaModeForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 69);
    static final int TRANSACTION_getAllCellInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 70);
    static final int TRANSACTION_requestCellInfoUpdate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 71);
    static final int TRANSACTION_requestCellInfoUpdateWithWorkSource = (android.os.IBinder.FIRST_CALL_TRANSACTION + 72);
    static final int TRANSACTION_setCellInfoListRate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 73);
    static final int TRANSACTION_iccOpenLogicalChannelBySlot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 74);
    static final int TRANSACTION_iccOpenLogicalChannel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 75);
    static final int TRANSACTION_iccCloseLogicalChannelBySlot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 76);
    static final int TRANSACTION_iccCloseLogicalChannel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 77);
    static final int TRANSACTION_iccTransmitApduLogicalChannelBySlot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 78);
    static final int TRANSACTION_iccTransmitApduLogicalChannel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 79);
    static final int TRANSACTION_iccTransmitApduBasicChannelBySlot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 80);
    static final int TRANSACTION_iccTransmitApduBasicChannel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 81);
    static final int TRANSACTION_iccExchangeSimIO = (android.os.IBinder.FIRST_CALL_TRANSACTION + 82);
    static final int TRANSACTION_sendEnvelopeWithStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 83);
    static final int TRANSACTION_nvReadItem = (android.os.IBinder.FIRST_CALL_TRANSACTION + 84);
    static final int TRANSACTION_nvWriteItem = (android.os.IBinder.FIRST_CALL_TRANSACTION + 85);
    static final int TRANSACTION_nvWriteCdmaPrl = (android.os.IBinder.FIRST_CALL_TRANSACTION + 86);
    static final int TRANSACTION_resetModemConfig = (android.os.IBinder.FIRST_CALL_TRANSACTION + 87);
    static final int TRANSACTION_rebootModem = (android.os.IBinder.FIRST_CALL_TRANSACTION + 88);
    static final int TRANSACTION_getCalculatedPreferredNetworkType = (android.os.IBinder.FIRST_CALL_TRANSACTION + 89);
    static final int TRANSACTION_getPreferredNetworkType = (android.os.IBinder.FIRST_CALL_TRANSACTION + 90);
    static final int TRANSACTION_getTetherApnRequiredForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 91);
    static final int TRANSACTION_enableIms = (android.os.IBinder.FIRST_CALL_TRANSACTION + 92);
    static final int TRANSACTION_disableIms = (android.os.IBinder.FIRST_CALL_TRANSACTION + 93);
    static final int TRANSACTION_getMmTelFeatureAndListen = (android.os.IBinder.FIRST_CALL_TRANSACTION + 94);
    static final int TRANSACTION_getRcsFeatureAndListen = (android.os.IBinder.FIRST_CALL_TRANSACTION + 95);
    static final int TRANSACTION_getImsRegistration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 96);
    static final int TRANSACTION_getImsConfig = (android.os.IBinder.FIRST_CALL_TRANSACTION + 97);
    static final int TRANSACTION_setImsService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 98);
    static final int TRANSACTION_getImsService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 99);
    static final int TRANSACTION_setNetworkSelectionModeAutomatic = (android.os.IBinder.FIRST_CALL_TRANSACTION + 100);
    static final int TRANSACTION_getCellNetworkScanResults = (android.os.IBinder.FIRST_CALL_TRANSACTION + 101);
    static final int TRANSACTION_requestNetworkScan = (android.os.IBinder.FIRST_CALL_TRANSACTION + 102);
    static final int TRANSACTION_stopNetworkScan = (android.os.IBinder.FIRST_CALL_TRANSACTION + 103);
    static final int TRANSACTION_setNetworkSelectionModeManual = (android.os.IBinder.FIRST_CALL_TRANSACTION + 104);
    static final int TRANSACTION_setPreferredNetworkType = (android.os.IBinder.FIRST_CALL_TRANSACTION + 105);
    static final int TRANSACTION_setUserDataEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 106);
    static final int TRANSACTION_getDataEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 107);
    static final int TRANSACTION_isUserDataEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 108);
    static final int TRANSACTION_isDataEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 109);
    static final int TRANSACTION_isManualNetworkSelectionAllowed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 110);
    static final int TRANSACTION_getPcscfAddress = (android.os.IBinder.FIRST_CALL_TRANSACTION + 111);
    static final int TRANSACTION_setImsRegistrationState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 112);
    static final int TRANSACTION_getCdmaMdn = (android.os.IBinder.FIRST_CALL_TRANSACTION + 113);
    static final int TRANSACTION_getCdmaMin = (android.os.IBinder.FIRST_CALL_TRANSACTION + 114);
    static final int TRANSACTION_requestNumberVerification = (android.os.IBinder.FIRST_CALL_TRANSACTION + 115);
    static final int TRANSACTION_getCarrierPrivilegeStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 116);
    static final int TRANSACTION_getCarrierPrivilegeStatusForUid = (android.os.IBinder.FIRST_CALL_TRANSACTION + 117);
    static final int TRANSACTION_checkCarrierPrivilegesForPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 118);
    static final int TRANSACTION_checkCarrierPrivilegesForPackageAnyPhone = (android.os.IBinder.FIRST_CALL_TRANSACTION + 119);
    static final int TRANSACTION_getCarrierPackageNamesForIntentAndPhone = (android.os.IBinder.FIRST_CALL_TRANSACTION + 120);
    static final int TRANSACTION_setLine1NumberForDisplayForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 121);
    static final int TRANSACTION_getLine1NumberForDisplay = (android.os.IBinder.FIRST_CALL_TRANSACTION + 122);
    static final int TRANSACTION_getLine1AlphaTagForDisplay = (android.os.IBinder.FIRST_CALL_TRANSACTION + 123);
    static final int TRANSACTION_getMergedSubscriberIds = (android.os.IBinder.FIRST_CALL_TRANSACTION + 124);
    static final int TRANSACTION_getMergedSubscriberIdsFromGroup = (android.os.IBinder.FIRST_CALL_TRANSACTION + 125);
    static final int TRANSACTION_setOperatorBrandOverride = (android.os.IBinder.FIRST_CALL_TRANSACTION + 126);
    static final int TRANSACTION_setRoamingOverride = (android.os.IBinder.FIRST_CALL_TRANSACTION + 127);
    static final int TRANSACTION_invokeOemRilRequestRaw = (android.os.IBinder.FIRST_CALL_TRANSACTION + 128);
    static final int TRANSACTION_needMobileRadioShutdown = (android.os.IBinder.FIRST_CALL_TRANSACTION + 129);
    static final int TRANSACTION_shutdownMobileRadios = (android.os.IBinder.FIRST_CALL_TRANSACTION + 130);
    static final int TRANSACTION_setRadioCapability = (android.os.IBinder.FIRST_CALL_TRANSACTION + 131);
    static final int TRANSACTION_getRadioAccessFamily = (android.os.IBinder.FIRST_CALL_TRANSACTION + 132);
    static final int TRANSACTION_enableVideoCalling = (android.os.IBinder.FIRST_CALL_TRANSACTION + 133);
    static final int TRANSACTION_isVideoCallingEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 134);
    static final int TRANSACTION_canChangeDtmfToneLength = (android.os.IBinder.FIRST_CALL_TRANSACTION + 135);
    static final int TRANSACTION_isWorldPhone = (android.os.IBinder.FIRST_CALL_TRANSACTION + 136);
    static final int TRANSACTION_isTtyModeSupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 137);
    static final int TRANSACTION_isRttSupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 138);
    static final int TRANSACTION_isHearingAidCompatibilitySupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 139);
    static final int TRANSACTION_isImsRegistered = (android.os.IBinder.FIRST_CALL_TRANSACTION + 140);
    static final int TRANSACTION_isWifiCallingAvailable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 141);
    static final int TRANSACTION_isVideoTelephonyAvailable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 142);
    static final int TRANSACTION_getImsRegTechnologyForMmTel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 143);
    static final int TRANSACTION_getDeviceId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 144);
    static final int TRANSACTION_getImeiForSlot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 145);
    static final int TRANSACTION_getTypeAllocationCodeForSlot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 146);
    static final int TRANSACTION_getMeidForSlot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 147);
    static final int TRANSACTION_getManufacturerCodeForSlot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 148);
    static final int TRANSACTION_getDeviceSoftwareVersionForSlot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 149);
    static final int TRANSACTION_getSubIdForPhoneAccount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 150);
    static final int TRANSACTION_getPhoneAccountHandleForSubscriptionId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 151);
    static final int TRANSACTION_factoryReset = (android.os.IBinder.FIRST_CALL_TRANSACTION + 152);
    static final int TRANSACTION_getSimLocaleForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 153);
    static final int TRANSACTION_requestModemActivityInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 154);
    static final int TRANSACTION_getServiceStateForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 155);
    static final int TRANSACTION_getVoicemailRingtoneUri = (android.os.IBinder.FIRST_CALL_TRANSACTION + 156);
    static final int TRANSACTION_setVoicemailRingtoneUri = (android.os.IBinder.FIRST_CALL_TRANSACTION + 157);
    static final int TRANSACTION_isVoicemailVibrationEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 158);
    static final int TRANSACTION_setVoicemailVibrationEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 159);
    static final int TRANSACTION_getPackagesWithCarrierPrivileges = (android.os.IBinder.FIRST_CALL_TRANSACTION + 160);
    static final int TRANSACTION_getPackagesWithCarrierPrivilegesForAllPhones = (android.os.IBinder.FIRST_CALL_TRANSACTION + 161);
    static final int TRANSACTION_getAidForAppType = (android.os.IBinder.FIRST_CALL_TRANSACTION + 162);
    static final int TRANSACTION_getEsn = (android.os.IBinder.FIRST_CALL_TRANSACTION + 163);
    static final int TRANSACTION_getCdmaPrlVersion = (android.os.IBinder.FIRST_CALL_TRANSACTION + 164);
    static final int TRANSACTION_getTelephonyHistograms = (android.os.IBinder.FIRST_CALL_TRANSACTION + 165);
    static final int TRANSACTION_setAllowedCarriers = (android.os.IBinder.FIRST_CALL_TRANSACTION + 166);
    static final int TRANSACTION_getAllowedCarriers = (android.os.IBinder.FIRST_CALL_TRANSACTION + 167);
    static final int TRANSACTION_getSubscriptionCarrierId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 168);
    static final int TRANSACTION_getSubscriptionCarrierName = (android.os.IBinder.FIRST_CALL_TRANSACTION + 169);
    static final int TRANSACTION_getSubscriptionSpecificCarrierId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 170);
    static final int TRANSACTION_getSubscriptionSpecificCarrierName = (android.os.IBinder.FIRST_CALL_TRANSACTION + 171);
    static final int TRANSACTION_getCarrierIdFromMccMnc = (android.os.IBinder.FIRST_CALL_TRANSACTION + 172);
    static final int TRANSACTION_carrierActionSetMeteredApnsEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 173);
    static final int TRANSACTION_carrierActionSetRadioEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 174);
    static final int TRANSACTION_carrierActionReportDefaultNetworkStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 175);
    static final int TRANSACTION_carrierActionResetAll = (android.os.IBinder.FIRST_CALL_TRANSACTION + 176);
    static final int TRANSACTION_getVtDataUsage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 177);
    static final int TRANSACTION_setPolicyDataEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 178);
    static final int TRANSACTION_getClientRequestStats = (android.os.IBinder.FIRST_CALL_TRANSACTION + 179);
    static final int TRANSACTION_setSimPowerStateForSlot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 180);
    static final int TRANSACTION_getForbiddenPlmns = (android.os.IBinder.FIRST_CALL_TRANSACTION + 181);
    static final int TRANSACTION_getEmergencyCallbackMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 182);
    static final int TRANSACTION_getSignalStrength = (android.os.IBinder.FIRST_CALL_TRANSACTION + 183);
    static final int TRANSACTION_getCardIdForDefaultEuicc = (android.os.IBinder.FIRST_CALL_TRANSACTION + 184);
    static final int TRANSACTION_getUiccCardsInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 185);
    static final int TRANSACTION_getUiccSlotsInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 186);
    static final int TRANSACTION_switchSlots = (android.os.IBinder.FIRST_CALL_TRANSACTION + 187);
    static final int TRANSACTION_setRadioIndicationUpdateMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 188);
    static final int TRANSACTION_isDataRoamingEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 189);
    static final int TRANSACTION_setDataRoamingEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 190);
    static final int TRANSACTION_getCdmaRoamingMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 191);
    static final int TRANSACTION_setCdmaRoamingMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 192);
    static final int TRANSACTION_setCdmaSubscriptionMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 193);
    static final int TRANSACTION_setCarrierTestOverride = (android.os.IBinder.FIRST_CALL_TRANSACTION + 194);
    static final int TRANSACTION_getCarrierIdListVersion = (android.os.IBinder.FIRST_CALL_TRANSACTION + 195);
    static final int TRANSACTION_refreshUiccProfile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 196);
    static final int TRANSACTION_getNumberOfModemsWithSimultaneousDataConnections = (android.os.IBinder.FIRST_CALL_TRANSACTION + 197);
    static final int TRANSACTION_getNetworkSelectionMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 198);
    static final int TRANSACTION_isInEmergencySmsMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 199);
    static final int TRANSACTION_getSmsApps = (android.os.IBinder.FIRST_CALL_TRANSACTION + 200);
    static final int TRANSACTION_getDefaultSmsApp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 201);
    static final int TRANSACTION_setDefaultSmsApp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 202);
    static final int TRANSACTION_getRadioPowerState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 203);
    static final int TRANSACTION_registerImsRegistrationCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 204);
    static final int TRANSACTION_unregisterImsRegistrationCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 205);
    static final int TRANSACTION_registerMmTelCapabilityCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 206);
    static final int TRANSACTION_unregisterMmTelCapabilityCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 207);
    static final int TRANSACTION_isCapable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 208);
    static final int TRANSACTION_isAvailable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 209);
    static final int TRANSACTION_isAdvancedCallingSettingEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 210);
    static final int TRANSACTION_setAdvancedCallingSettingEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 211);
    static final int TRANSACTION_isVtSettingEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 212);
    static final int TRANSACTION_setVtSettingEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 213);
    static final int TRANSACTION_isVoWiFiSettingEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 214);
    static final int TRANSACTION_setVoWiFiSettingEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 215);
    static final int TRANSACTION_isVoWiFiRoamingSettingEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 216);
    static final int TRANSACTION_setVoWiFiRoamingSettingEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 217);
    static final int TRANSACTION_setVoWiFiNonPersistent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 218);
    static final int TRANSACTION_getVoWiFiModeSetting = (android.os.IBinder.FIRST_CALL_TRANSACTION + 219);
    static final int TRANSACTION_setVoWiFiModeSetting = (android.os.IBinder.FIRST_CALL_TRANSACTION + 220);
    static final int TRANSACTION_getVoWiFiRoamingModeSetting = (android.os.IBinder.FIRST_CALL_TRANSACTION + 221);
    static final int TRANSACTION_setVoWiFiRoamingModeSetting = (android.os.IBinder.FIRST_CALL_TRANSACTION + 222);
    static final int TRANSACTION_setRttCapabilitySetting = (android.os.IBinder.FIRST_CALL_TRANSACTION + 223);
    static final int TRANSACTION_isTtyOverVolteEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 224);
    static final int TRANSACTION_getEmergencyNumberList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 225);
    static final int TRANSACTION_isEmergencyNumber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 226);
    static final int TRANSACTION_getCertsFromCarrierPrivilegeAccessRules = (android.os.IBinder.FIRST_CALL_TRANSACTION + 227);
    static final int TRANSACTION_registerImsProvisioningChangedCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 228);
    static final int TRANSACTION_unregisterImsProvisioningChangedCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 229);
    static final int TRANSACTION_setImsProvisioningStatusForCapability = (android.os.IBinder.FIRST_CALL_TRANSACTION + 230);
    static final int TRANSACTION_getImsProvisioningStatusForCapability = (android.os.IBinder.FIRST_CALL_TRANSACTION + 231);
    static final int TRANSACTION_isMmTelCapabilityProvisionedInCache = (android.os.IBinder.FIRST_CALL_TRANSACTION + 232);
    static final int TRANSACTION_cacheMmTelCapabilityProvisioning = (android.os.IBinder.FIRST_CALL_TRANSACTION + 233);
    static final int TRANSACTION_getImsProvisioningInt = (android.os.IBinder.FIRST_CALL_TRANSACTION + 234);
    static final int TRANSACTION_getImsProvisioningString = (android.os.IBinder.FIRST_CALL_TRANSACTION + 235);
    static final int TRANSACTION_setImsProvisioningInt = (android.os.IBinder.FIRST_CALL_TRANSACTION + 236);
    static final int TRANSACTION_setImsProvisioningString = (android.os.IBinder.FIRST_CALL_TRANSACTION + 237);
    static final int TRANSACTION_updateEmergencyNumberListTestMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 238);
    static final int TRANSACTION_getEmergencyNumberListTestMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 239);
    static final int TRANSACTION_enableModemForSlot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 240);
    static final int TRANSACTION_setMultiSimCarrierRestriction = (android.os.IBinder.FIRST_CALL_TRANSACTION + 241);
    static final int TRANSACTION_isMultiSimSupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 242);
    static final int TRANSACTION_switchMultiSimConfig = (android.os.IBinder.FIRST_CALL_TRANSACTION + 243);
    static final int TRANSACTION_doesSwitchMultiSimConfigTriggerReboot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 244);
    static final int TRANSACTION_getSlotsMapping = (android.os.IBinder.FIRST_CALL_TRANSACTION + 245);
    static final int TRANSACTION_getRadioHalVersion = (android.os.IBinder.FIRST_CALL_TRANSACTION + 246);
    static final int TRANSACTION_isModemEnabledForSlot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 247);
    static final int TRANSACTION_isDataEnabledForApn = (android.os.IBinder.FIRST_CALL_TRANSACTION + 248);
    static final int TRANSACTION_isApnMetered = (android.os.IBinder.FIRST_CALL_TRANSACTION + 249);
    static final int TRANSACTION_enqueueSmsPickResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 250);
    static final int TRANSACTION_getMmsUserAgent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 251);
    static final int TRANSACTION_getMmsUAProfUrl = (android.os.IBinder.FIRST_CALL_TRANSACTION + 252);
    static final int TRANSACTION_setDataAllowedDuringVoiceCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 253);
    static final int TRANSACTION_isDataAllowedInVoiceCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 254);
    public static boolean setDefaultImpl(com.android.internal.telephony.ITelephony impl) {
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
    public static com.android.internal.telephony.ITelephony getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Dial a number. This doesn't place the call. It displays
       * the Dialer screen.
       * @param number the number to be dialed. If null, this
       * would display the Dialer screen with no number pre-filled.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/ITelephony.aidl:81:1:81:25")
  public void dial(java.lang.String number) throws android.os.RemoteException;
  /**
       * Place a call to the specified number.
       * @param callingPackage The package making the call.
       * @param number the number to be called.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/ITelephony.aidl:89:1:89:25")
  public void call(java.lang.String callingPackage, java.lang.String number) throws android.os.RemoteException;
  /**
       * Check to see if the radio is on or not.
       * @param callingPackage the name of the package making the call.
       * @return returns true if the radio is on.
       */
  public boolean isRadioOn(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Check to see if the radio is on or not on particular subId.
       * @param subId user preferred subId.
       * @param callingPackage the name of the package making the call.
       * @return returns true if the radio is on.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/ITelephony.aidl:105:1:105:25")
  public boolean isRadioOnForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Supply a pin to unlock the SIM.  Blocks until a result is determined.
       * @param pin The pin to check.
       * @return whether the operation was a success.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/ITelephony.aidl:113:1:113:25")
  public boolean supplyPin(java.lang.String pin) throws android.os.RemoteException;
  /**
       * Supply a pin to unlock the SIM for particular subId.
       * Blocks until a result is determined.
       * @param pin The pin to check.
       * @param subId user preferred subId.
       * @return whether the operation was a success.
       */
  public boolean supplyPinForSubscriber(int subId, java.lang.String pin) throws android.os.RemoteException;
  /**
       * Supply puk to unlock the SIM and set SIM pin to new pin.
       *  Blocks until a result is determined.
       * @param puk The puk to check.
       *        pin The new pin to be set in SIM
       * @return whether the operation was a success.
       */
  public boolean supplyPuk(java.lang.String puk, java.lang.String pin) throws android.os.RemoteException;
  /**
       * Supply puk to unlock the SIM and set SIM pin to new pin.
       *  Blocks until a result is determined.
       * @param puk The puk to check.
       *        pin The new pin to be set in SIM
       * @param subId user preferred subId.
       * @return whether the operation was a success.
       */
  public boolean supplyPukForSubscriber(int subId, java.lang.String puk, java.lang.String pin) throws android.os.RemoteException;
  /**
       * Supply a pin to unlock the SIM.  Blocks until a result is determined.
       * Returns a specific success/error code.
       * @param pin The pin to check.
       * @return retValue[0] = Phone.PIN_RESULT_SUCCESS on success. Otherwise error code
       *         retValue[1] = number of attempts remaining if known otherwise -1
       */
  public int[] supplyPinReportResult(java.lang.String pin) throws android.os.RemoteException;
  /**
       * Supply a pin to unlock the SIM.  Blocks until a result is determined.
       * Returns a specific success/error code.
       * @param pin The pin to check.
       * @return retValue[0] = Phone.PIN_RESULT_SUCCESS on success. Otherwise error code
       *         retValue[1] = number of attempts remaining if known otherwise -1
       */
  public int[] supplyPinReportResultForSubscriber(int subId, java.lang.String pin) throws android.os.RemoteException;
  /**
       * Supply puk to unlock the SIM and set SIM pin to new pin.
       * Blocks until a result is determined.
       * Returns a specific success/error code
       * @param puk The puk to check
       *        pin The pin to check.
       * @return retValue[0] = Phone.PIN_RESULT_SUCCESS on success. Otherwise error code
       *         retValue[1] = number of attempts remaining if known otherwise -1
       */
  public int[] supplyPukReportResult(java.lang.String puk, java.lang.String pin) throws android.os.RemoteException;
  /**
       * Supply puk to unlock the SIM and set SIM pin to new pin.
       * Blocks until a result is determined.
       * Returns a specific success/error code
       * @param puk The puk to check
       *        pin The pin to check.
       * @return retValue[0] = Phone.PIN_RESULT_SUCCESS on success. Otherwise error code
       *         retValue[1] = number of attempts remaining if known otherwise -1
       */
  public int[] supplyPukReportResultForSubscriber(int subId, java.lang.String puk, java.lang.String pin) throws android.os.RemoteException;
  /**
       * Handles PIN MMI commands (PIN/PIN2/PUK/PUK2), which are initiated
       * without SEND (so <code>dial</code> is not appropriate).
       *
       * @param dialString the MMI command to be executed.
       * @return true if MMI command is executed.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/ITelephony.aidl:191:1:191:25")
  public boolean handlePinMmi(java.lang.String dialString) throws android.os.RemoteException;
  /**
       * Handles USSD commands.
       *
       * @param subId The subscription to use.
       * @param ussdRequest the USSD command to be executed.
       * @param wrappedCallback receives a callback result.
       */
  public void handleUssdRequest(int subId, java.lang.String ussdRequest, android.os.ResultReceiver wrappedCallback) throws android.os.RemoteException;
  /**
       * Handles PIN MMI commands (PIN/PIN2/PUK/PUK2), which are initiated
       * without SEND (so <code>dial</code> is not appropriate) for
       * a particular subId.
       * @param dialString the MMI command to be executed.
       * @param subId user preferred subId.
       * @return true if MMI command is executed.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/ITelephony.aidl:212:1:212:25")
  public boolean handlePinMmiForSubscriber(int subId, java.lang.String dialString) throws android.os.RemoteException;
  /**
       * Toggles the radio on or off.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/ITelephony.aidl:218:1:218:25")
  public void toggleRadioOnOff() throws android.os.RemoteException;
  /**
       * Toggles the radio on or off on particular subId.
       * @param subId user preferred subId.
       */
  public void toggleRadioOnOffForSubscriber(int subId) throws android.os.RemoteException;
  /**
       * Set the radio to on or off
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/ITelephony.aidl:230:1:230:25")
  public boolean setRadio(boolean turnOn) throws android.os.RemoteException;
  /**
       * Set the radio to on or off on particular subId.
       * @param subId user preferred subId.
       */
  public boolean setRadioForSubscriber(int subId, boolean turnOn) throws android.os.RemoteException;
  /**
       * Set the radio to on or off unconditionally
       */
  public boolean setRadioPower(boolean turnOn) throws android.os.RemoteException;
  /**
       * Request to update location information in service state
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/ITelephony.aidl:247:1:247:25")
  public void updateServiceLocation() throws android.os.RemoteException;
  /**
       * Request to update location information for a subscrition in service state
       * @param subId user preferred subId.
       */
  public void updateServiceLocationForSubscriber(int subId) throws android.os.RemoteException;
  /**
       * Enable location update notifications.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/ITelephony.aidl:259:1:259:25")
  public void enableLocationUpdates() throws android.os.RemoteException;
  /**
       * Enable location update notifications.
       * @param subId user preferred subId.
       */
  public void enableLocationUpdatesForSubscriber(int subId) throws android.os.RemoteException;
  /**
       * Disable location update notifications.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/ITelephony.aidl:271:1:271:25")
  public void disableLocationUpdates() throws android.os.RemoteException;
  /**
       * Disable location update notifications.
       * @param subId user preferred subId.
       */
  public void disableLocationUpdatesForSubscriber(int subId) throws android.os.RemoteException;
  /**
       * Allow mobile data connections.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/ITelephony.aidl:283:1:283:25")
  public boolean enableDataConnectivity() throws android.os.RemoteException;
  /**
       * Disallow mobile data connections.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/ITelephony.aidl:289:1:289:25")
  public boolean disableDataConnectivity() throws android.os.RemoteException;
  /**
       * Report whether data connectivity is possible.
       */
  public boolean isDataConnectivityPossible(int subId) throws android.os.RemoteException;
  public android.os.Bundle getCellLocation(java.lang.String callingPkg) throws android.os.RemoteException;
  /**
       * Returns the ISO country code equivalent of the current registered
       * operator's MCC (Mobile Country Code).
       * @see android.telephony.TelephonyManager#getNetworkCountryIso
       */
  public java.lang.String getNetworkCountryIsoForPhone(int phoneId) throws android.os.RemoteException;
  /**
       * Returns the neighboring cell information of the device.
       */
  public java.util.List<android.telephony.NeighboringCellInfo> getNeighboringCellInfo(java.lang.String callingPkg) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/ITelephony.aidl:311:1:311:25")
  public int getCallState() throws android.os.RemoteException;
  /**
       * Returns the call state for a slot.
       */
  public int getCallStateForSlot(int slotIndex) throws android.os.RemoteException;
  /**
       * Replaced by getDataActivityForSubId.
       */
  public int getDataActivity() throws android.os.RemoteException;
  /**
       * Returns a constant indicating the type of activity on a data connection
       * (cellular).
       *
       * @see #DATA_ACTIVITY_NONE
       * @see #DATA_ACTIVITY_IN
       * @see #DATA_ACTIVITY_OUT
       * @see #DATA_ACTIVITY_INOUT
       * @see #DATA_ACTIVITY_DORMANT
       */
  public int getDataActivityForSubId(int subId) throws android.os.RemoteException;
  /**
       * Replaced by getDataStateForSubId.
       */
  public int getDataState() throws android.os.RemoteException;
  /**
       * Returns a constant indicating the current data connection state
       * (cellular).
       *
       * @see #DATA_DISCONNECTED
       * @see #DATA_CONNECTING
       * @see #DATA_CONNECTED
       * @see #DATA_SUSPENDED
       */
  public int getDataStateForSubId(int subId) throws android.os.RemoteException;
  /**
       * Returns the current active phone type as integer.
       * Returns TelephonyManager.PHONE_TYPE_CDMA if RILConstants.CDMA_PHONE
       * and TelephonyManager.PHONE_TYPE_GSM if RILConstants.GSM_PHONE
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/ITelephony.aidl:357:1:357:25")
  public int getActivePhoneType() throws android.os.RemoteException;
  /**
       * Returns the current active phone type as integer for particular slot.
       * Returns TelephonyManager.PHONE_TYPE_CDMA if RILConstants.CDMA_PHONE
       * and TelephonyManager.PHONE_TYPE_GSM if RILConstants.GSM_PHONE
       * @param slotIndex - slot to query.
       */
  public int getActivePhoneTypeForSlot(int slotIndex) throws android.os.RemoteException;
  /**
       * Returns the CDMA ERI icon index to display
       * @param callingPackage package making the call.
       */
  public int getCdmaEriIconIndex(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Returns the CDMA ERI icon index to display on particular subId.
       * @param subId user preferred subId.
       * @param callingPackage package making the call.
       */
  public int getCdmaEriIconIndexForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Returns the CDMA ERI icon mode,
       * 0 - ON
       * 1 - FLASHING
       * @param callingPackage package making the call.
       */
  public int getCdmaEriIconMode(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Returns the CDMA ERI icon mode on particular subId,
       * 0 - ON
       * 1 - FLASHING
       * @param subId user preferred subId.
       * @param callingPackage package making the call.
       */
  public int getCdmaEriIconModeForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Returns the CDMA ERI text,
       * @param callingPackage package making the call.
       */
  public java.lang.String getCdmaEriText(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Returns the CDMA ERI text for particular subId,
       * @param subId user preferred subId.
       * @param callingPackage package making the call.
       */
  public java.lang.String getCdmaEriTextForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Returns true if OTA service provisioning needs to run.
       * Only relevant on some technologies, others will always
       * return false.
       */
  public boolean needsOtaServiceProvisioning() throws android.os.RemoteException;
  /**
       * Sets the voicemail number for a particular subscriber.
       */
  public boolean setVoiceMailNumber(int subId, java.lang.String alphaTag, java.lang.String number) throws android.os.RemoteException;
  /**
        * Sets the voice activation state for a particular subscriber.
        */
  public void setVoiceActivationState(int subId, int activationState) throws android.os.RemoteException;
  /**
        * Sets the data activation state for a particular subscriber.
        */
  public void setDataActivationState(int subId, int activationState) throws android.os.RemoteException;
  /**
        * Returns the voice activation state for a particular subscriber.
        * @param subId user preferred sub
        * @param callingPackage package queries voice activation state
        */
  public int getVoiceActivationState(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
        * Returns the data activation state for a particular subscriber.
        * @param subId user preferred sub
        * @param callingPackage package queris data activation state
        */
  public int getDataActivationState(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Returns the unread count of voicemails for a subId.
       * @param subId user preferred subId.
       * Returns the unread count of voicemails
       */
  public int getVoiceMessageCountForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
        * Returns true if current state supports both voice and data
        * simultaneously. This can change based on location or network condition.
        */
  public boolean isConcurrentVoiceAndDataAllowed(int subId) throws android.os.RemoteException;
  public android.os.Bundle getVisualVoicemailSettings(java.lang.String callingPackage, int subId) throws android.os.RemoteException;
  public java.lang.String getVisualVoicemailPackageName(java.lang.String callingPackage, int subId) throws android.os.RemoteException;
  // Not oneway, caller needs to make sure the vaule is set before receiving a SMS

  public void enableVisualVoicemailSmsFilter(java.lang.String callingPackage, int subId, android.telephony.VisualVoicemailSmsFilterSettings settings) throws android.os.RemoteException;
  public void disableVisualVoicemailSmsFilter(java.lang.String callingPackage, int subId) throws android.os.RemoteException;
  // Get settings set by the calling package

  public android.telephony.VisualVoicemailSmsFilterSettings getVisualVoicemailSmsFilterSettings(java.lang.String callingPackage, int subId) throws android.os.RemoteException;
  /**
       *  Get settings set by the current default dialer, Internal use only.
       *  Requires READ_PRIVILEGED_PHONE_STATE permission.
       */
  public android.telephony.VisualVoicemailSmsFilterSettings getActiveVisualVoicemailSmsFilterSettings(int subId) throws android.os.RemoteException;
  /**
       * Send a visual voicemail SMS. Internal use only.
       * Requires caller to be the default dialer and have SEND_SMS permission
       */
  public void sendVisualVoicemailSmsForSubscriber(java.lang.String callingPackage, int subId, java.lang.String number, int port, java.lang.String text, android.app.PendingIntent sentIntent) throws android.os.RemoteException;
  // Send the special dialer code. The IPC caller must be the current default dialer.

  public void sendDialerSpecialCode(java.lang.String callingPackageName, java.lang.String inputCode) throws android.os.RemoteException;
  /**
       * Returns the network type of a subId.
       * @param subId user preferred subId.
       * @param callingPackage package making the call.
       */
  public int getNetworkTypeForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Returns the network type for data transmission
       * @param callingPackage package making the call.
       */
  public int getDataNetworkType(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Returns the data network type of a subId
       * @param subId user preferred subId.
       * @param callingPackage package making the call.
       */
  public int getDataNetworkTypeForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
        * Returns the voice network type of a subId
        * @param subId user preferred subId.
        * @param callingPackage package making the call.
        * Returns the network type
        */
  public int getVoiceNetworkTypeForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Return true if an ICC card is present
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/ITelephony.aidl:521:1:521:25")
  public boolean hasIccCard() throws android.os.RemoteException;
  /**
       * Return true if an ICC card is present for a subId.
       * @param slotIndex user preferred slotIndex.
       * Return true if an ICC card is present
       */
  public boolean hasIccCardUsingSlotIndex(int slotIndex) throws android.os.RemoteException;
  /**
       * Return if the current radio is LTE on CDMA. This
       * is a tri-state return value as for a period of time
       * the mode may be unknown.
       *
       * @param callingPackage the name of the calling package
       * @return {@link Phone#LTE_ON_CDMA_UNKNOWN}, {@link Phone#LTE_ON_CDMA_FALSE}
       * or {@link PHone#LTE_ON_CDMA_TRUE}
       */
  public int getLteOnCdmaMode(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Return if the current radio is LTE on CDMA. This
       * is a tri-state return value as for a period of time
       * the mode may be unknown.
       *
       * @param callingPackage the name of the calling package
       * @return {@link Phone#LTE_ON_CDMA_UNKNOWN}, {@link Phone#LTE_ON_CDMA_FALSE}
       * or {@link PHone#LTE_ON_CDMA_TRUE}
       */
  public int getLteOnCdmaModeForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Returns all observed cell information of the device.
       */
  public java.util.List<android.telephony.CellInfo> getAllCellInfo(java.lang.String callingPkg) throws android.os.RemoteException;
  /**
       * Request a cell information update for the specified subscription,
       * reported via the CellInfoCallback.
       */
  public void requestCellInfoUpdate(int subId, android.telephony.ICellInfoCallback cb, java.lang.String callingPkg) throws android.os.RemoteException;
  /**
       * Request a cell information update for the specified subscription,
       * reported via the CellInfoCallback.
       *
       * @param workSource the requestor to whom the power consumption for this should be attributed.
       */
  public void requestCellInfoUpdateWithWorkSource(int subId, android.telephony.ICellInfoCallback cb, java.lang.String callingPkg, android.os.WorkSource ws) throws android.os.RemoteException;
  /**
       * Sets minimum time in milli-seconds between onCellInfoChanged
       */
  public void setCellInfoListRate(int rateInMillis) throws android.os.RemoteException;
  /**
       * Opens a logical channel to the ICC card using the physical slot index.
       *
       * Input parameters equivalent to TS 27.007 AT+CCHO command.
       *
       * @param slotIndex The physical slot index of the target ICC card
       * @param callingPackage the name of the package making the call.
       * @param AID Application id. See ETSI 102.221 and 101.220.
       * @param p2 P2 parameter (described in ISO 7816-4).
       * @return an IccOpenLogicalChannelResponse object.
       */
  public android.telephony.IccOpenLogicalChannelResponse iccOpenLogicalChannelBySlot(int slotIndex, java.lang.String callingPackage, java.lang.String AID, int p2) throws android.os.RemoteException;
  /**
       * Opens a logical channel to the ICC card.
       *
       * Input parameters equivalent to TS 27.007 AT+CCHO command.
       *
       * @param subId The subscription to use.
       * @param callingPackage the name of the package making the call.
       * @param AID Application id. See ETSI 102.221 and 101.220.
       * @param p2 P2 parameter (described in ISO 7816-4).
       * @return an IccOpenLogicalChannelResponse object.
       */
  public android.telephony.IccOpenLogicalChannelResponse iccOpenLogicalChannel(int subId, java.lang.String callingPackage, java.lang.String AID, int p2) throws android.os.RemoteException;
  /**
       * Closes a previously opened logical channel to the ICC card using the physical slot index.
       *
       * Input parameters equivalent to TS 27.007 AT+CCHC command.
       *
       * @param slotIndex The physical slot index of the target ICC card
       * @param channel is the channel id to be closed as returned by a
       *            successful iccOpenLogicalChannel.
       * @return true if the channel was closed successfully.
       */
  public boolean iccCloseLogicalChannelBySlot(int slotIndex, int channel) throws android.os.RemoteException;
  /**
       * Closes a previously opened logical channel to the ICC card.
       *
       * Input parameters equivalent to TS 27.007 AT+CCHC command.
       *
       * @param subId The subscription to use.
       * @param channel is the channel id to be closed as returned by a
       *            successful iccOpenLogicalChannel.
       * @return true if the channel was closed successfully.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/ITelephony.aidl:628:1:628:25")
  public boolean iccCloseLogicalChannel(int subId, int channel) throws android.os.RemoteException;
  /**
       * Transmit an APDU to the ICC card over a logical channel using the physical slot index.
       *
       * Input parameters equivalent to TS 27.007 AT+CGLA command.
       *
       * @param slotIndex The physical slot index of the target ICC card
       * @param channel is the channel id to be closed as returned by a
       *            successful iccOpenLogicalChannel.
       * @param cla Class of the APDU command.
       * @param instruction Instruction of the APDU command.
       * @param p1 P1 value of the APDU command.
       * @param p2 P2 value of the APDU command.
       * @param p3 P3 value of the APDU command. If p3 is negative a 4 byte APDU
       *            is sent to the SIM.
       * @param data Data to be sent with the APDU.
       * @return The APDU response from the ICC card with the status appended at
       *            the end.
       */
  public java.lang.String iccTransmitApduLogicalChannelBySlot(int slotIndex, int channel, int cla, int instruction, int p1, int p2, int p3, java.lang.String data) throws android.os.RemoteException;
  /**
       * Transmit an APDU to the ICC card over a logical channel.
       *
       * Input parameters equivalent to TS 27.007 AT+CGLA command.
       *
       * @param subId The subscription to use.
       * @param channel is the channel id to be closed as returned by a
       *            successful iccOpenLogicalChannel.
       * @param cla Class of the APDU command.
       * @param instruction Instruction of the APDU command.
       * @param p1 P1 value of the APDU command.
       * @param p2 P2 value of the APDU command.
       * @param p3 P3 value of the APDU command. If p3 is negative a 4 byte APDU
       *            is sent to the SIM.
       * @param data Data to be sent with the APDU.
       * @return The APDU response from the ICC card with the status appended at
       *            the end.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/ITelephony.aidl:670:1:670:25")
  public java.lang.String iccTransmitApduLogicalChannel(int subId, int channel, int cla, int instruction, int p1, int p2, int p3, java.lang.String data) throws android.os.RemoteException;
  /**
       * Transmit an APDU to the ICC card over the basic channel using the physical slot index.
       *
       * Input parameters equivalent to TS 27.007 AT+CSIM command.
       *
       * @param slotIndex The physical slot index of the target ICC card
       * @param callingPackage the name of the package making the call.
       * @param cla Class of the APDU command.
       * @param instruction Instruction of the APDU command.
       * @param p1 P1 value of the APDU command.
       * @param p2 P2 value of the APDU command.
       * @param p3 P3 value of the APDU command. If p3 is negative a 4 byte APDU
       *            is sent to the SIM.
       * @param data Data to be sent with the APDU.
       * @return The APDU response from the ICC card with the status appended at
       *            the end.
       */
  public java.lang.String iccTransmitApduBasicChannelBySlot(int slotIndex, java.lang.String callingPackage, int cla, int instruction, int p1, int p2, int p3, java.lang.String data) throws android.os.RemoteException;
  /**
       * Transmit an APDU to the ICC card over the basic channel.
       *
       * Input parameters equivalent to TS 27.007 AT+CSIM command.
       *
       * @param subId The subscription to use.
       * @param callingPackage the name of the package making the call.
       * @param cla Class of the APDU command.
       * @param instruction Instruction of the APDU command.
       * @param p1 P1 value of the APDU command.
       * @param p2 P2 value of the APDU command.
       * @param p3 P3 value of the APDU command. If p3 is negative a 4 byte APDU
       *            is sent to the SIM.
       * @param data Data to be sent with the APDU.
       * @return The APDU response from the ICC card with the status appended at
       *            the end.
       */
  public java.lang.String iccTransmitApduBasicChannel(int subId, java.lang.String callingPackage, int cla, int instruction, int p1, int p2, int p3, java.lang.String data) throws android.os.RemoteException;
  /**
       * Returns the response APDU for a command APDU sent through SIM_IO.
       *
       * @param subId The subscription to use.
       * @param fileID
       * @param command
       * @param p1 P1 value of the APDU command.
       * @param p2 P2 value of the APDU command.
       * @param p3 P3 value of the APDU command.
       * @param filePath
       * @return The APDU response.
       */
  public byte[] iccExchangeSimIO(int subId, int fileID, int command, int p1, int p2, int p3, java.lang.String filePath) throws android.os.RemoteException;
  /**
       * Send ENVELOPE to the SIM and returns the response.
       *
       * @param subId The subscription to use.
       * @param contents  String containing SAT/USAT response in hexadecimal
       *                  format starting with command tag. See TS 102 223 for
       *                  details.
       * @return The APDU response from the ICC card, with the last 4 bytes
       *         being the status word. If the command fails, returns an empty
       *         string.
       */
  public java.lang.String sendEnvelopeWithStatus(int subId, java.lang.String content) throws android.os.RemoteException;
  /**
       * Read one of the NV items defined in {@link RadioNVItems} / {@code ril_nv_items.h}.
       * Used for device configuration by some CDMA operators.
       *
       * @param itemID the ID of the item to read.
       * @return the NV item as a String, or null on any failure.
       */
  public java.lang.String nvReadItem(int itemID) throws android.os.RemoteException;
  /**
       * Write one of the NV items defined in {@link RadioNVItems} / {@code ril_nv_items.h}.
       * Used for device configuration by some CDMA operators.
       *
       * @param itemID the ID of the item to read.
       * @param itemValue the value to write, as a String.
       * @return true on success; false on any failure.
       */
  public boolean nvWriteItem(int itemID, java.lang.String itemValue) throws android.os.RemoteException;
  /**
       * Update the CDMA Preferred Roaming List (PRL) in the radio NV storage.
       * Used for device configuration by some CDMA operators.
       *
       * @param preferredRoamingList byte array containing the new PRL.
       * @return true on success; false on any failure.
       */
  public boolean nvWriteCdmaPrl(byte[] preferredRoamingList) throws android.os.RemoteException;
  /**
       * Rollback modem configurations to factory default except some config which are in whitelist.
       * Used for device configuration by some CDMA operators.
       *
       * <p>Requires Permission:
       * {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE} or that the calling
       * app has carrier privileges (see {@link #hasCarrierPrivileges}).
       *
       * @param slotIndex - device slot.
       * @return {@code true} on success; {@code false} on any failure.
       */
  public boolean resetModemConfig(int slotIndex) throws android.os.RemoteException;
  /**
       * Generate a radio modem reset. Used for device configuration by some CDMA operators.
       * Different than {@link #setRadioPower(boolean)}, modem reboot will power down sim card.
       *
       * <p>Requires Permission:
       * {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE} or that the calling
       * app has carrier privileges (see {@link #hasCarrierPrivileges}).
       *
       * @param slotIndex - device slot.
       * @return {@code true} on success; {@code false} on any failure.
       */
  public boolean rebootModem(int slotIndex) throws android.os.RemoteException;
  /*
       * Get the calculated preferred network type.
       * Used for device configuration by some CDMA operators.
       * @param callingPackage The package making the call.
       *
       * @return the calculated preferred network type, defined in RILConstants.java.
       */
  public int getCalculatedPreferredNetworkType(java.lang.String callingPackage) throws android.os.RemoteException;
  /*
       * Get the preferred network type.
       * Used for device configuration by some CDMA operators.
       *
       * @param subId the id of the subscription to query.
       * @return the preferred network type, defined in RILConstants.java.
       */
  public int getPreferredNetworkType(int subId) throws android.os.RemoteException;
  /**
       * Check whether DUN APN is required for tethering with subId.
       *
       * @param subId the id of the subscription to require tethering.
       * @return {@code true} if DUN APN is required for tethering.
       * @hide
       */
  public boolean getTetherApnRequiredForSubscriber(int subId) throws android.os.RemoteException;
  /**
      * Enables framework IMS and triggers IMS Registration.
      */
  public void enableIms(int slotId) throws android.os.RemoteException;
  /**
      * Disables framework IMS and triggers IMS deregistration.
      */
  public void disableIms(int slotId) throws android.os.RemoteException;
  /**
       *  Get IImsMmTelFeature binder from ImsResolver that corresponds to the subId and MMTel feature
       *  as well as registering the MmTelFeature for callbacks using the IImsServiceFeatureCallback
       *  interface.
       */
  public android.telephony.ims.aidl.IImsMmTelFeature getMmTelFeatureAndListen(int slotId, com.android.ims.internal.IImsServiceFeatureCallback callback) throws android.os.RemoteException;
  /**
       *  Get IImsRcsFeature binder from ImsResolver that corresponds to the subId and RCS feature
       *  as well as registering the RcsFeature for callbacks using the IImsServiceFeatureCallback
       *  interface.
       */
  public android.telephony.ims.aidl.IImsRcsFeature getRcsFeatureAndListen(int slotId, com.android.ims.internal.IImsServiceFeatureCallback callback) throws android.os.RemoteException;
  /**
      * Returns the IImsRegistration associated with the slot and feature specified.
      */
  public android.telephony.ims.aidl.IImsRegistration getImsRegistration(int slotId, int feature) throws android.os.RemoteException;
  /**
      * Returns the IImsConfig associated with the slot and feature specified.
      */
  public android.telephony.ims.aidl.IImsConfig getImsConfig(int slotId, int feature) throws android.os.RemoteException;
  /**
      *  @return true if the ImsService to bind to for the slot id specified was set, false otherwise.
      */
  public boolean setImsService(int slotId, boolean isCarrierImsService, java.lang.String packageName) throws android.os.RemoteException;
  /**
      * @return the package name of the carrier/device ImsService associated with this slot.
      */
  public java.lang.String getImsService(int slotId, boolean isCarrierImsService) throws android.os.RemoteException;
  /**
       * Set the network selection mode to automatic.
       *
       * @param subId the id of the subscription to update.
       */
  public void setNetworkSelectionModeAutomatic(int subId) throws android.os.RemoteException;
  /**
       * Perform a radio scan and return the list of avialble networks.
       *
       * @param subId the id of the subscription.
       * @return CellNetworkScanResult containing status of scan and networks.
       */
  public com.android.internal.telephony.CellNetworkScanResult getCellNetworkScanResults(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Perform a radio network scan and return the id of this scan.
       *
       * @param subId the id of the subscription.
       * @param request Defines all the configs for network scan.
       * @param messenger Callback messages will be sent using this messenger.
       * @param binder the binder object instantiated in TelephonyManager.
       * @param callingPackage the calling package
       * @return An id for this scan.
       */
  public int requestNetworkScan(int subId, android.telephony.NetworkScanRequest request, android.os.Messenger messenger, android.os.IBinder binder, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Stop an existing radio network scan.
       *
       * @param subId the id of the subscription.
       * @param scanId The id of the scan that is going to be stopped.
       */
  public void stopNetworkScan(int subId, int scanId) throws android.os.RemoteException;
  /**
       * Ask the radio to connect to the input network and change selection mode to manual.
       *
       * @param subId the id of the subscription.
       * @param operatorInfo the operator inforamtion, included the PLMN, long name and short name of
       * the operator to attach to.
       * @param persistSelection whether the selection will persist until reboot. If true, only allows
       * attaching to the selected PLMN until reboot; otherwise, attach to the chosen PLMN and resume
       * normal network selection next time.
       * @return {@code true} on success; {@code true} on any failure.
       */
  public boolean setNetworkSelectionModeManual(int subId, com.android.internal.telephony.OperatorInfo operatorInfo, boolean persisSelection) throws android.os.RemoteException;
  /**
       * Set the preferred network type.
       * Used for device configuration by some CDMA operators.
       *
       * @param subId the id of the subscription to update.
       * @param networkType the preferred network type, defined in RILConstants.java.
       * @return true on success; false on any failure.
       */
  public boolean setPreferredNetworkType(int subId, int networkType) throws android.os.RemoteException;
  /**
       * User enable/disable Mobile Data.
       *
       * @param enable true to turn on, else false
       */
  public void setUserDataEnabled(int subId, boolean enable) throws android.os.RemoteException;
  /**
       * Get the user enabled state of Mobile Data.
       *
       * TODO: remove and use isUserDataEnabled.
       * This can't be removed now because some vendor codes
       * calls through ITelephony directly while they should
       * use TelephonyManager.
       *
       * @return true on enabled
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/ITelephony.aidl:943:1:943:25")
  public boolean getDataEnabled(int subId) throws android.os.RemoteException;
  /**
       * Get the user enabled state of Mobile Data.
       *
       * @return true on enabled
       */
  public boolean isUserDataEnabled(int subId) throws android.os.RemoteException;
  /**
       * Get the overall enabled state of Mobile Data.
       *
       * @return true on enabled
       */
  public boolean isDataEnabled(int subId) throws android.os.RemoteException;
  /**
       * Checks if manual network selection is allowed.
       *
       * @return {@code true} if manual network selection is allowed, otherwise return {@code false}.
       */
  public boolean isManualNetworkSelectionAllowed(int subId) throws android.os.RemoteException;
  /**
       * Get P-CSCF address from PCO after data connection is established or modified.
       * @param apnType the apnType, "ims" for IMS APN, "emergency" for EMERGENCY APN
       * @param callingPackage The package making the call.
       */
  public java.lang.String[] getPcscfAddress(java.lang.String apnType, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Set IMS registration state
       */
  public void setImsRegistrationState(boolean registered) throws android.os.RemoteException;
  /**
       * Return MDN string for CDMA phone.
       * @param subId user preferred subId.
       */
  public java.lang.String getCdmaMdn(int subId) throws android.os.RemoteException;
  /**
       * Return MIN string for CDMA phone.
       * @param subId user preferred subId.
       */
  public java.lang.String getCdmaMin(int subId) throws android.os.RemoteException;
  /**
       * Request that the next incoming call from a number matching {@code range} be intercepted.
       * @param range The range of phone numbers the caller expects a phone call from.
       * @param timeoutMillis The amount of time to wait for such a call, or
       *                      {@link #MAX_NUMBER_VERIFICATION_TIMEOUT_MILLIS}, whichever is lesser.
       * @param callback the callback aidl
       * @param callingPackage the calling package name.
       */
  public void requestNumberVerification(android.telephony.PhoneNumberRange range, long timeoutMillis, com.android.internal.telephony.INumberVerificationCallback callback, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Has the calling application been granted special privileges by the carrier.
       *
       * If any of the packages in the calling UID has carrier privileges, the
       * call will return true. This access is granted by the owner of the UICC
       * card and does not depend on the registered carrier.
       *
       * TODO: Add a link to documentation.
       *
       * @param subId The subscription to use.
       * @return carrier privilege status defined in TelephonyManager.
       */
  public int getCarrierPrivilegeStatus(int subId) throws android.os.RemoteException;
  /**
       * Similar to above, but check for the given uid.
       */
  public int getCarrierPrivilegeStatusForUid(int subId, int uid) throws android.os.RemoteException;
  /**
       * Similar to above, but check for the package whose name is pkgName.
       */
  public int checkCarrierPrivilegesForPackage(int subId, java.lang.String pkgName) throws android.os.RemoteException;
  /**
       * Similar to above, but check across all phones.
       */
  public int checkCarrierPrivilegesForPackageAnyPhone(java.lang.String pkgName) throws android.os.RemoteException;
  /**
       * Returns list of the package names of the carrier apps that should handle the input intent
       * and have carrier privileges for the given phoneId.
       *
       * @param intent Intent that will be sent.
       * @param phoneId The phoneId on which the carrier app has carrier privileges.
       * @return list of carrier app package names that can handle the intent on phoneId.
       *         Returns null if there is an error and an empty list if there
       *         are no matching packages.
       */
  public java.util.List<java.lang.String> getCarrierPackageNamesForIntentAndPhone(android.content.Intent intent, int phoneId) throws android.os.RemoteException;
  /**
       * Set the line 1 phone number string and its alphatag for the current ICCID
       * for display purpose only, for example, displayed in Phone Status. It won't
       * change the actual MSISDN/MDN. To unset alphatag or number, pass in a null
       * value.
       *
       * @param subId the subscriber that the alphatag and dialing number belongs to.
       * @param alphaTag alpha-tagging of the dailing nubmer
       * @param number The dialing number
       * @return true if the operation was executed correctly.
       */
  public boolean setLine1NumberForDisplayForSubscriber(int subId, java.lang.String alphaTag, java.lang.String number) throws android.os.RemoteException;
  /**
       * Returns the displayed dialing number string if it was set previously via
       * {@link #setLine1NumberForDisplay}. Otherwise returns null.
       *
       * @param subId whose dialing number for line 1 is returned.
       * @param callingPackage The package making the call.
       * @return the displayed dialing number if set, or null if not set.
       */
  public java.lang.String getLine1NumberForDisplay(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Returns the displayed alphatag of the dialing number if it was set
       * previously via {@link #setLine1NumberForDisplay}. Otherwise returns null.
       *
       * @param subId whose alphatag associated with line 1 is returned.
       * @param callingPackage The package making the call.
       * @return the displayed alphatag of the dialing number if set, or null if
       *         not set.
       */
  public java.lang.String getLine1AlphaTagForDisplay(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Return the set of subscriber IDs that should be considered "merged together" for data usage
       * purposes. This is commonly {@code null} to indicate no merging is required. Any returned
       * subscribers are sorted in a deterministic order.
       * <p>
       * The returned set of subscriber IDs will include the subscriber ID corresponding to this
       * TelephonyManager's subId.
       *
       * @hide
       */
  public java.lang.String[] getMergedSubscriberIds(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @hide
       */
  public java.lang.String[] getMergedSubscriberIdsFromGroup(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Override the operator branding for the current ICCID.
       *
       * Once set, whenever the SIM is present in the device, the service
       * provider name (SPN) and the operator name will both be replaced by the
       * brand value input. To unset the value, the same function should be
       * called with a null brand value.
       *
       * <p>Requires Permission:
       *   {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}
       *  or has to be carrier app - see #hasCarrierPrivileges.
       *
       * @param subId The subscription to use.
       * @param brand The brand name to display/set.
       * @return true if the operation was executed correctly.
       */
  public boolean setOperatorBrandOverride(int subId, java.lang.String brand) throws android.os.RemoteException;
  /**
       * Override the roaming indicator for the current ICCID.
       *
       * Using this call, the carrier app (see #hasCarrierPrivileges) can override
       * the platform's notion of a network operator being considered roaming or not.
       * The change only affects the ICCID that was active when this call was made.
       *
       * If null is passed as any of the input, the corresponding value is deleted.
       *
       * <p>Requires that the caller have carrier privilege. See #hasCarrierPrivileges.
       *
       * @param subId for which the roaming overrides apply.
       * @param gsmRoamingList - List of MCCMNCs to be considered roaming for 3GPP RATs.
       * @param gsmNonRoamingList - List of MCCMNCs to be considered not roaming for 3GPP RATs.
       * @param cdmaRoamingList - List of SIDs to be considered roaming for 3GPP2 RATs.
       * @param cdmaNonRoamingList - List of SIDs to be considered not roaming for 3GPP2 RATs.
       * @return true if the operation was executed correctly.
       */
  public boolean setRoamingOverride(int subId, java.util.List<java.lang.String> gsmRoamingList, java.util.List<java.lang.String> gsmNonRoamingList, java.util.List<java.lang.String> cdmaRoamingList, java.util.List<java.lang.String> cdmaNonRoamingList) throws android.os.RemoteException;
  /**
       * Returns the result and response from RIL for oem request
       *
       * @param oemReq the data is sent to ril.
       * @param oemResp the respose data from RIL.
       * @return negative value request was not handled or get error
       *         0 request was handled succesfully, but no response data
       *         positive value success, data length of response
       */
  public int invokeOemRilRequestRaw(byte[] oemReq, byte[] oemResp) throws android.os.RemoteException;
  /**
       * Check if any mobile Radios need to be shutdown.
       *
       * @return true is any mobile radio needs to be shutdown
       */
  public boolean needMobileRadioShutdown() throws android.os.RemoteException;
  /**
       * Shutdown Mobile Radios
       */
  public void shutdownMobileRadios() throws android.os.RemoteException;
  /**
       * Set phone radio type and access technology.
       *
       * @param rafs an RadioAccessFamily array to indicate all phone's
       *        new radio access family. The length of RadioAccessFamily
       *        must equ]]al to phone count.
       */
  public void setRadioCapability(android.telephony.RadioAccessFamily[] rafs) throws android.os.RemoteException;
  /**
       * Get phone radio type and access technology.
       *
       * @param phoneId which phone you want to get
       * @param callingPackage the name of the package making the call
       * @return phone radio type and access technology
       */
  public int getRadioAccessFamily(int phoneId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Enables or disables video calling.
       *
       * @param enable Whether to enable video calling.
       */
  public void enableVideoCalling(boolean enable) throws android.os.RemoteException;
  /**
       * Whether video calling has been enabled by the user.
       *
       * @param callingPackage The package making the call.
       * @return {@code true} if the user has enabled video calling, {@code false} otherwise.
       */
  public boolean isVideoCallingEnabled(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Whether the DTMF tone length can be changed.
       *
       * @param subId The subscription to use.
       * @param callingPackage The package making the call.
       * @return {@code true} if the DTMF tone length can be changed.
       */
  public boolean canChangeDtmfToneLength(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Whether the device is a world phone.
       *
       * @param callingPackage The package making the call.
       * @return {@code true} if the devices is a world phone.
       */
  public boolean isWorldPhone(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Whether the phone supports TTY mode.
       *
       * @return {@code true} if the device supports TTY mode.
       */
  public boolean isTtyModeSupported() throws android.os.RemoteException;
  public boolean isRttSupported(int subscriptionId) throws android.os.RemoteException;
  /**
       * Whether the phone supports hearing aid compatibility.
       *
       * @return {@code true} if the device supports hearing aid compatibility.
       */
  public boolean isHearingAidCompatibilitySupported() throws android.os.RemoteException;
  /**
       * Get IMS Registration Status on a particular subid.
       *
       * @param subId user preferred subId.
       *
       * @return {@code true} if the IMS status is registered.
       */
  public boolean isImsRegistered(int subId) throws android.os.RemoteException;
  /**
       * Returns the Status of Wi-Fi Calling for the subscription id specified.
       */
  public boolean isWifiCallingAvailable(int subId) throws android.os.RemoteException;
  /**
       * Returns the Status of VT (video telephony) for the subscription ID specified.
       */
  public boolean isVideoTelephonyAvailable(int subId) throws android.os.RemoteException;
  /**
      * Returns the MMTEL IMS registration technology for the subsciption ID specified.
      */
  public int getImsRegTechnologyForMmTel(int subId) throws android.os.RemoteException;
  /**
        * Returns the unique device ID of phone, for example, the IMEI for
        * GSM and the MEID for CDMA phones. Return null if device ID is not available.
        *
        * @param callingPackage The package making the call.
        * <p>Requires Permission:
        *   {@link android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE}
        */
  public java.lang.String getDeviceId(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Returns the IMEI for the given slot.
       *
       * @param slotIndex - device slot.
       * @param callingPackage The package making the call.
       * <p>Requires Permission:
       *   {@link android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE}
       */
  public java.lang.String getImeiForSlot(int slotIndex, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Returns the Type Allocation Code from the IMEI for the given slot.
       *
       * @param slotIndex - Which slot to retrieve the Type Allocation Code from.
       */
  public java.lang.String getTypeAllocationCodeForSlot(int slotIndex) throws android.os.RemoteException;
  /**
       * Returns the MEID for the given slot.
       *
       * @param slotIndex - device slot.
       * @param callingPackage The package making the call.
       * <p>Requires Permission:
       *   {@link android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE}
       */
  public java.lang.String getMeidForSlot(int slotIndex, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Returns the Manufacturer Code from the MEID for the given slot.
       *
       * @param slotIndex - Which slot to retrieve the Manufacturer Code from.
       */
  public java.lang.String getManufacturerCodeForSlot(int slotIndex) throws android.os.RemoteException;
  /**
       * Returns the device software version.
       *
       * @param slotIndex - device slot.
       * @param callingPackage The package making the call.
       * <p>Requires Permission:
       *   {@link android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE}
       */
  public java.lang.String getDeviceSoftwareVersionForSlot(int slotIndex, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Returns the subscription ID associated with the specified PhoneAccount.
       */
  public int getSubIdForPhoneAccount(android.telecom.PhoneAccount phoneAccount) throws android.os.RemoteException;
  /**
       * Returns the PhoneAccountHandle associated with a subscription ID.
       */
  public android.telecom.PhoneAccountHandle getPhoneAccountHandleForSubscriptionId(int subscriptionId) throws android.os.RemoteException;
  public void factoryReset(int subId) throws android.os.RemoteException;
  /**
       * Returns users's current locale based on the SIM.
       *
       * The returned string will be a well formed BCP-47 language tag, or {@code null}
       * if no locale could be derived.
       */
  public java.lang.String getSimLocaleForSubscriber(int subId) throws android.os.RemoteException;
  /**
       * Requests the modem activity info asynchronously.
       * The implementor is expected to reply with the
       * {@link android.telephony.ModemActivityInfo} object placed into the Bundle with the key
       * {@link android.telephony.TelephonyManager#MODEM_ACTIVITY_RESULT_KEY}.
       * The result code is ignored.
       */
  public void requestModemActivityInfo(android.os.ResultReceiver result) throws android.os.RemoteException;
  /**
       * Get the service state on specified subscription
       * @param subId Subscription id
       * @param callingPackage The package making the call
       * @return Service state on specified subscription.
       */
  public android.telephony.ServiceState getServiceStateForSubscriber(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Returns the URI for the per-account voicemail ringtone set in Phone settings.
       *
       * @param accountHandle The handle for the {@link PhoneAccount} for which to retrieve the
       * voicemail ringtone.
       * @return The URI for the ringtone to play when receiving a voicemail from a specific
       * PhoneAccount.
       */
  public android.net.Uri getVoicemailRingtoneUri(android.telecom.PhoneAccountHandle accountHandle) throws android.os.RemoteException;
  /**
       * Sets the per-account voicemail ringtone.
       *
       * <p>Requires that the calling app is the default dialer, or has carrier privileges, or
       * has permission {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}.
       *
       * @param phoneAccountHandle The handle for the {@link PhoneAccount} for which to set the
       * voicemail ringtone.
       * @param uri The URI for the ringtone to play when receiving a voicemail from a specific
       * PhoneAccount.
       */
  public void setVoicemailRingtoneUri(java.lang.String callingPackage, android.telecom.PhoneAccountHandle phoneAccountHandle, android.net.Uri uri) throws android.os.RemoteException;
  /**
       * Returns whether vibration is set for voicemail notification in Phone settings.
       *
       * @param accountHandle The handle for the {@link PhoneAccount} for which to retrieve the
       * voicemail vibration setting.
       * @return {@code true} if the vibration is set for this PhoneAccount, {@code false} otherwise.
       */
  public boolean isVoicemailVibrationEnabled(android.telecom.PhoneAccountHandle accountHandle) throws android.os.RemoteException;
  /**
       * Sets the per-account preference whether vibration is enabled for voicemail notifications.
       *
       * <p>Requires that the calling app is the default dialer, or has carrier privileges, or
       * has permission {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}.
       *
       * @param phoneAccountHandle The handle for the {@link PhoneAccount} for which to set the
       * voicemail vibration setting.
       * @param enabled Whether to enable or disable vibration for voicemail notifications from a
       * specific PhoneAccount.
       */
  public void setVoicemailVibrationEnabled(java.lang.String callingPackage, android.telecom.PhoneAccountHandle phoneAccountHandle, boolean enabled) throws android.os.RemoteException;
  /**
       * Returns a list of packages that have carrier privileges for the specific phone.
       */
  public java.util.List<java.lang.String> getPackagesWithCarrierPrivileges(int phoneId) throws android.os.RemoteException;
  /**
        * Returns a list of packages that have carrier privileges.
        */
  public java.util.List<java.lang.String> getPackagesWithCarrierPrivilegesForAllPhones() throws android.os.RemoteException;
  /**
       * Return the application ID for the app type.
       *
       * @param subId the subscription ID that this request applies to.
       * @param appType the uicc app type,
       * @return Application ID for specificied app type or null if no uicc or error.
       */
  public java.lang.String getAidForAppType(int subId, int appType) throws android.os.RemoteException;
  /**
      * Return the Electronic Serial Number.
      *
      * Requires that the calling app has READ_PRIVILEGED_PHONE_STATE permission
      *
      * @param subId the subscription ID that this request applies to.
      * @return ESN or null if error.
      * @hide
      */
  public java.lang.String getEsn(int subId) throws android.os.RemoteException;
  /**
      * Return the Preferred Roaming List Version
      *
      * Requires that the calling app has READ_PRIVILEGED_PHONE_STATE permission
      * @param subId the subscription ID that this request applies to.
      * @return PRLVersion or null if error.
      * @hide
      */
  public java.lang.String getCdmaPrlVersion(int subId) throws android.os.RemoteException;
  /**
       * Get snapshot of Telephony histograms
       * @return List of Telephony histograms
       * Requires Permission:
       *   {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}
       * Or the calling app has carrier privileges.
       */
  public java.util.List<android.telephony.TelephonyHistogram> getTelephonyHistograms() throws android.os.RemoteException;
  /**
       * Set the allowed carrier list and the excluded carrier list, indicating the priority between
       * the two lists.
       *
       * <p>Requires system privileges. In the future we may add this to carrier APIs.
       *
       * @return {@link #SET_CARRIER_RESTRICTION_SUCCESS} in case of success.
       * {@link #SET_CARRIER_RESTRICTION_NOT_SUPPORTED} if the modem does not support the
       * configuration. {@link #SET_CARRIER_RESTRICTION_ERROR} in all other error cases.
       */
  public int setAllowedCarriers(android.telephony.CarrierRestrictionRules carrierRestrictionRules) throws android.os.RemoteException;
  /**
       * Get the allowed carrier list and the excluded carrier list indicating the priority between
       * the two lists.
       *
       * <p>Requires system privileges. In the future we may add this to carrier APIs.
       *
       * @return {@link CarrierRestrictionRules}; empty lists mean all carriers are allowed. It
       * returns null in case of error.
       */
  public android.telephony.CarrierRestrictionRules getAllowedCarriers() throws android.os.RemoteException;
  /**
       * Returns carrier id of the given subscription.
       * <p>To recognize carrier as a first class identity, assign each carrier with a canonical
       * integer a.k.a carrier id.
       *
       * @param subId The subscription id
       * @return Carrier id of given subscription id. return {@link #UNKNOWN_CARRIER_ID} if
       * subscription is unavailable or carrier cannot be identified.
       * @throws IllegalStateException if telephony service is unavailable.
       * @hide
       */
  public int getSubscriptionCarrierId(int subId) throws android.os.RemoteException;
  /**
       * Returns carrier name of the given subscription.
       * <p>Carrier name is a user-facing name of carrier id {@link #getSimCarrierId(int)},
       * usually the brand name of the subsidiary (e.g. T-Mobile). Each carrier could configure
       * multiple {@link #getSimOperatorName() SPN} but should have a single carrier name.
       * Carrier name is not canonical identity, use {@link #getSimCarrierId(int)} instead.
       * <p>Returned carrier name is unlocalized.
       *
       * @return Carrier name of given subscription id. return {@code null} if subscription is
       * unavailable or carrier cannot be identified.
       * @throws IllegalStateException if telephony service is unavailable.
       * @hide
       */
  public java.lang.String getSubscriptionCarrierName(int subId) throws android.os.RemoteException;
  /**
       * Returns fine-grained carrier id of the current subscription.
       *
       * <p>The specific carrier id can be used to further differentiate a carrier by different
       * networks, by prepaid v.s.postpaid or even by 4G v.s.3G plan. Each carrier has a unique
       * carrier id {@link #getSimCarrierId()} but can have multiple precise carrier id. e.g,
       * {@link #getSimCarrierId()} will always return Tracfone (id 2022) for a Tracfone SIM, while
       * {@link #getSimPreciseCarrierId()} can return Tracfone AT&T or Tracfone T-Mobile based on the
       * current underlying network.
       *
       * <p>For carriers without any fine-grained carrier ids, return {@link #getSimCarrierId()}
       *
       * @return Returns fine-grained carrier id of the current subscription.
       * Return {@link #UNKNOWN_CARRIER_ID} if the subscription is unavailable or the carrier cannot
       * be identified.
       * @hide
       */
  public int getSubscriptionSpecificCarrierId(int subId) throws android.os.RemoteException;
  /**
       * Similar like {@link #getSimCarrierIdName()}, returns user-facing name of the
       * specific carrier id {@link #getSimSpecificCarrierId()}
       *
       * <p>The returned name is unlocalized.
       *
       * @return user-facing name of the subscription specific carrier id. Return {@code null} if the
       * subscription is unavailable or the carrier cannot be identified.
       * @hide
       */
  public java.lang.String getSubscriptionSpecificCarrierName(int subId) throws android.os.RemoteException;
  /**
       * Returns carrier id based on MCCMNC only. This will return a MNO carrier id used for fallback
       * check when exact carrier id {@link #getSimCarrierId()} configurations are not found
       *
       * @param isSubscriptionMccMnc. If {@true} it means this is a query for subscription mccmnc
       * {@false} otherwise.
       *
       * @return carrier id from passing mccmnc.
       * @hide
       */
  public int getCarrierIdFromMccMnc(int slotIndex, java.lang.String mccmnc, boolean isSubscriptionMccMnc) throws android.os.RemoteException;
  /**
       * Action set from carrier signalling broadcast receivers to enable/disable metered apns
       * Permissions android.Manifest.permission.MODIFY_PHONE_STATE is required
       * @param subId the subscription ID that this action applies to.
       * @param enabled control enable or disable metered apns.
       * @hide
       */
  public void carrierActionSetMeteredApnsEnabled(int subId, boolean visible) throws android.os.RemoteException;
  /**
       * Action set from carrier signalling broadcast receivers to enable/disable radio
       * Permissions android.Manifest.permission.MODIFY_PHONE_STATE is required
       * @param subId the subscription ID that this action applies to.
       * @param enabled control enable or disable radio.
       * @hide
       */
  public void carrierActionSetRadioEnabled(int subId, boolean enabled) throws android.os.RemoteException;
  /**
       * Action set from carrier signalling broadcast receivers to start/stop reporting default
       * network conditions.
       * Permissions android.Manifest.permission.MODIFY_PHONE_STATE is required
       * @param subId the subscription ID that this action applies to.
       * @param report control start/stop reporting default network events.
       * @hide
       */
  public void carrierActionReportDefaultNetworkStatus(int subId, boolean report) throws android.os.RemoteException;
  /**
       * Action set from carrier signalling broadcast receivers to reset all carrier actions.
       * Permissions android.Manifest.permission.MODIFY_PHONE_STATE is required
       * @param subId the subscription ID that this action applies to.
       * @hide
       */
  public void carrierActionResetAll(int subId) throws android.os.RemoteException;
  /**
       * Get aggregated video call data usage since boot.
       * Permissions android.Manifest.permission.READ_NETWORK_USAGE_HISTORY is required.
       *
       * @param perUidStats True if requesting data usage per uid, otherwise overall usage.
       * @return Snapshot of video call data usage
       * @hide
       */
  public android.net.NetworkStats getVtDataUsage(int subId, boolean perUidStats) throws android.os.RemoteException;
  /**
       * Policy control of data connection. Usually used when data limit is passed.
       * @param enabled True if enabling the data, otherwise disabling.
       * @param subId Subscription index
       * @hide
       */
  public void setPolicyDataEnabled(boolean enabled, int subId) throws android.os.RemoteException;
  /**
       * Get Client request stats which will contain statistical information
       * on each request made by client.
       * @param callingPackage package making the call.
       * @param subId Subscription index
       * @hide
       */
  public java.util.List<android.telephony.ClientRequestStats> getClientRequestStats(java.lang.String callingPackage, int subid) throws android.os.RemoteException;
  /**
       * Set SIM card power state.
       * @param slotIndex SIM slot id
       * @param state  State of SIM (power down, power up, pass through)
       * @hide
       * */
  public void setSimPowerStateForSlot(int slotIndex, int state) throws android.os.RemoteException;
  /**
       * Returns a list of Forbidden PLMNs from the specified SIM App
       * Returns null if the query fails.
       *
       * <p>Requires that the calling app has READ_PRIVILEGED_PHONE_STATE or READ_PHONE_STATE
       *
       * @param subId subscription ID used for authentication
       * @param appType the icc application type, like {@link #APPTYPE_USIM}
       */
  public java.lang.String[] getForbiddenPlmns(int subId, int appType, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Check if phone is in emergency callback mode
       * @return true if phone is in emergency callback mode
       * @param subId the subscription ID that this action applies to.
       * @hide
       */
  public boolean getEmergencyCallbackMode(int subId) throws android.os.RemoteException;
  /**
       * Get the most recently available signal strength information.
       *
       * Get the most recent SignalStrength information reported by the modem. Due
       * to power saving this information may not always be current.
       * @param subId Subscription index
       * @return the most recent cached signal strength info from the modem
       * @hide
       */
  public android.telephony.SignalStrength getSignalStrength(int subId) throws android.os.RemoteException;
  /**
       * Get the card ID of the default eUICC card. If there is no eUICC, returns
       * {@link #INVALID_CARD_ID}.
       *
       * @param subId subscription ID used for authentication
       * @param callingPackage package making the call
       * @return card ID of the default eUICC card.
       */
  public int getCardIdForDefaultEuicc(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Gets information about currently inserted UICCs and eUICCs.
       * <p>
       * Requires that the calling app has carrier privileges (see {@link #hasCarrierPrivileges}).
       * <p>
       * If the caller has carrier priviliges on any active subscription, then they have permission to
       * get simple information like the card ID ({@link UiccCardInfo#getCardId()}), whether the card
       * is an eUICC ({@link UiccCardInfo#isEuicc()}), and the slot index where the card is inserted
       * ({@link UiccCardInfo#getSlotIndex()}).
       * <p>
       * To get private information such as the EID ({@link UiccCardInfo#getEid()}) or ICCID
       * ({@link UiccCardInfo#getIccId()}), the caller must have carrier priviliges on that specific
       * UICC or eUICC card.
       * <p>
       * See {@link UiccCardInfo} for more details on the kind of information available.
       *
       * @param callingPackage package making the call, used to evaluate carrier privileges
       * @return a list of UiccCardInfo objects, representing information on the currently inserted
       * UICCs and eUICCs. Each UiccCardInfo in the list will have private information filtered out if
       * the caller does not have adequate permissions for that card.
       */
  public java.util.List<android.telephony.UiccCardInfo> getUiccCardsInfo(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Get slot info for all the UICC slots.
       * @return UiccSlotInfo array.
       * @hide
       */
  public android.telephony.UiccSlotInfo[] getUiccSlotsInfo() throws android.os.RemoteException;
  /**
       * Map logicalSlot to physicalSlot, and activate the physicalSlot if it is inactive.
       * @param physicalSlots Index i in the array representing physical slot for phone i. The array
       *        size should be same as getPhoneCount().
       * @return boolean Return true if the switch succeeds, false if the switch fails.
       */
  public boolean switchSlots(int[] physicalSlots) throws android.os.RemoteException;
  /**
       * Sets radio indication update mode. This can be used to control the behavior of indication
       * update from modem to Android frameworks. For example, by default several indication updates
       * are turned off when screen is off, but in some special cases (e.g. carkit is connected but
       * screen is off) we want to turn on those indications even when the screen is off.
       */
  public void setRadioIndicationUpdateMode(int subId, int filters, int mode) throws android.os.RemoteException;
  /**
       * Returns whether mobile data roaming is enabled on the subscription with id {@code subId}.
       *
       * @param subId the subscription id
       * @return {@code true} if the data roaming is enabled on this subscription.
       */
  public boolean isDataRoamingEnabled(int subId) throws android.os.RemoteException;
  /**
       * Enables/Disables the data roaming on the subscription with id {@code subId}.
       *
       * @param subId the subscription id
       * @param isEnabled {@code true} to enable mobile data roaming, otherwise disable it.
       */
  public void setDataRoamingEnabled(int subId, boolean isEnabled) throws android.os.RemoteException;
  /**
       * Gets the roaming mode for the CDMA phone with the subscription id {@code subId}.
       *
       * @param the subscription id.
       * @return the roaming mode for CDMA phone.
       */
  public int getCdmaRoamingMode(int subId) throws android.os.RemoteException;
  /**
       * Sets the roaming mode on the CDMA phone with the subscription {@code subId} to the given
       * roaming mode {@code mode}.
       *
       * @param subId the subscription id.
       * @param mode the roaming mode should be set.
       * @return {@code true} if successed.
       */
  public boolean setCdmaRoamingMode(int subId, int mode) throws android.os.RemoteException;
  /**
       * Sets the subscription mode for CDMA phone with the subscription {@code subId} to the given
       * subscription mode {@code mode}.
       *
       * @param subId the subscription id.
       * @param mode the subscription mode should be set.
       * @return {@code true} if successed.
       */
  public boolean setCdmaSubscriptionMode(int subId, int mode) throws android.os.RemoteException;
  /**
       * A test API to override carrier information including mccmnc, imsi, iccid, gid1, gid2,
       * plmn and spn. This would be handy for, eg, forcing a particular carrier id, carrier's config
       * (also any country or carrier overlays) to be loaded when using a test SIM with a call box.
       */
  public void setCarrierTestOverride(int subId, java.lang.String mccmnc, java.lang.String imsi, java.lang.String iccid, java.lang.String gid1, java.lang.String gid2, java.lang.String plmn, java.lang.String spn, java.lang.String carrierPrivilegeRules, java.lang.String apn) throws android.os.RemoteException;
  /**
       * A test API to return installed carrier id list version.
       */
  public int getCarrierIdListVersion(int subId) throws android.os.RemoteException;
  /**
       * A test API to reload the UICC profile.
       * @hide
       */
  public void refreshUiccProfile(int subId) throws android.os.RemoteException;
  /**
       * How many modems can have simultaneous data connections.
       * @hide
       */
  public int getNumberOfModemsWithSimultaneousDataConnections(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Return the network selection mode on the subscription with id {@code subId}.
       */
  public int getNetworkSelectionMode(int subId) throws android.os.RemoteException;
  /**
       * Return true if the device is in emergency sms mode, false otherwise.
       */
  public boolean isInEmergencySmsMode() throws android.os.RemoteException;
  /**
       * Get a list of SMS apps on a user.
       */
  public java.lang.String[] getSmsApps(int userId) throws android.os.RemoteException;
  /**
       * Get the default SMS app on a given user.
       */
  public java.lang.String getDefaultSmsApp(int userId) throws android.os.RemoteException;
  /**
       * Set the default SMS app to a given package on a given user.
       */
  public void setDefaultSmsApp(int userId, java.lang.String packageName) throws android.os.RemoteException;
  /**
       * Return the modem radio power state for slot index.
       *
       */
  public int getRadioPowerState(int slotIndex, java.lang.String callingPackage) throws android.os.RemoteException;
  // IMS specific AIDL commands, see ImsMmTelManager.java
  /**
       * Adds an IMS registration status callback for the subscription id specified.
       */
  public void registerImsRegistrationCallback(int subId, android.telephony.ims.aidl.IImsRegistrationCallback c) throws android.os.RemoteException;
  /**
        * Removes an existing IMS registration status callback for the subscription specified.
        */
  public void unregisterImsRegistrationCallback(int subId, android.telephony.ims.aidl.IImsRegistrationCallback c) throws android.os.RemoteException;
  /**
       * Adds an IMS MmTel capabilities callback for the subscription specified.
       */
  public void registerMmTelCapabilityCallback(int subId, android.telephony.ims.aidl.IImsCapabilityCallback c) throws android.os.RemoteException;
  /**
       * Removes an existing IMS MmTel capabilities callback for the subscription specified.
       */
  public void unregisterMmTelCapabilityCallback(int subId, android.telephony.ims.aidl.IImsCapabilityCallback c) throws android.os.RemoteException;
  /**
       * return true if the IMS MmTel capability for the given registration tech is capable.
       */
  public boolean isCapable(int subId, int capability, int regTech) throws android.os.RemoteException;
  /**
       * return true if the IMS MmTel capability for the given registration tech is available.
       */
  public boolean isAvailable(int subId, int capability, int regTech) throws android.os.RemoteException;
  /**
       * Returns true if the user's setting for 4G LTE is enabled, for the subscription specified.
       */
  public boolean isAdvancedCallingSettingEnabled(int subId) throws android.os.RemoteException;
  /**
       * Modify the user's setting for whether or not 4G LTE is enabled.
       */
  public void setAdvancedCallingSettingEnabled(int subId, boolean isEnabled) throws android.os.RemoteException;
  /**
       * return true if the user's setting for VT is enabled for the subscription.
       */
  public boolean isVtSettingEnabled(int subId) throws android.os.RemoteException;
  /**
       * Modify the user's setting for whether or not VT is available for the subscrption specified.
       */
  public void setVtSettingEnabled(int subId, boolean isEnabled) throws android.os.RemoteException;
  /**
       * return true if the user's setting for whether or not Voice over WiFi is currently enabled.
       */
  public boolean isVoWiFiSettingEnabled(int subId) throws android.os.RemoteException;
  /**
       * sets the user's setting for Voice over WiFi enabled state.
       */
  public void setVoWiFiSettingEnabled(int subId, boolean isEnabled) throws android.os.RemoteException;
  /**
       * return true if the user's setting for Voice over WiFi while roaming is enabled.
       */
  public boolean isVoWiFiRoamingSettingEnabled(int subId) throws android.os.RemoteException;
  /**
       * Sets the user's preference for whether or not Voice over WiFi is enabled for the current
       * subscription while roaming.
       */
  public void setVoWiFiRoamingSettingEnabled(int subId, boolean isEnabled) throws android.os.RemoteException;
  /**
       * Set the Voice over WiFi enabled state, but do not persist the setting.
       */
  public void setVoWiFiNonPersistent(int subId, boolean isCapable, int mode) throws android.os.RemoteException;
  /**
       * return the Voice over WiFi mode preference set by the user for the subscription specified.
       */
  public int getVoWiFiModeSetting(int subId) throws android.os.RemoteException;
  /**
       * sets the user's preference for the Voice over WiFi mode for the subscription specified.
       */
  public void setVoWiFiModeSetting(int subId, int mode) throws android.os.RemoteException;
  /**
       * return the Voice over WiFi mode preference set by the user for the subscription specified
       * while roaming.
       */
  public int getVoWiFiRoamingModeSetting(int subId) throws android.os.RemoteException;
  /**
       * sets the user's preference for the Voice over WiFi mode for the subscription specified
       * while roaming.
       */
  public void setVoWiFiRoamingModeSetting(int subId, int mode) throws android.os.RemoteException;
  /**
       * Modify the user's setting for whether or not RTT is enabled for the subscrption specified.
       */
  public void setRttCapabilitySetting(int subId, boolean isEnabled) throws android.os.RemoteException;
  /**
       * return true if TTY over VoLTE is enabled for the subscription specified.
       */
  public boolean isTtyOverVolteEnabled(int subId) throws android.os.RemoteException;
  /**
       * Return the emergency number list from all the active subscriptions.
       */
  public java.util.Map getEmergencyNumberList(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Identify if the number is emergency number, based on all the active subscriptions.
       */
  public boolean isEmergencyNumber(java.lang.String number, boolean exactMatch) throws android.os.RemoteException;
  /**
       * Return a list of certs in hex string from loaded carrier privileges access rules.
       */
  public java.util.List<java.lang.String> getCertsFromCarrierPrivilegeAccessRules(int subId) throws android.os.RemoteException;
  /**
       * Register an IMS provisioning change callback with Telephony.
       */
  public void registerImsProvisioningChangedCallback(int subId, android.telephony.ims.aidl.IImsConfigCallback callback) throws android.os.RemoteException;
  /**
       * unregister an existing IMS provisioning change callback.
       */
  public void unregisterImsProvisioningChangedCallback(int subId, android.telephony.ims.aidl.IImsConfigCallback callback) throws android.os.RemoteException;
  /**
       * Set the provisioning status for the IMS MmTel capability using the specified subscription.
       */
  public void setImsProvisioningStatusForCapability(int subId, int capability, int tech, boolean isProvisioned) throws android.os.RemoteException;
  /**
       * Get the provisioning status for the IMS MmTel capability specified.
       */
  public boolean getImsProvisioningStatusForCapability(int subId, int capability, int tech) throws android.os.RemoteException;
  /** Is the capability and tech flagged as provisioned in the cache */
  public boolean isMmTelCapabilityProvisionedInCache(int subId, int capability, int tech) throws android.os.RemoteException;
  /** Set the provisioning for the capability and tech in the cache */
  public void cacheMmTelCapabilityProvisioning(int subId, int capability, int tech, boolean isProvisioned) throws android.os.RemoteException;
  /**
       * Return an integer containing the provisioning value for the specified provisioning key.
       */
  public int getImsProvisioningInt(int subId, int key) throws android.os.RemoteException;
  /**
       * return a String containing the provisioning value for the provisioning key specified.
       */
  public java.lang.String getImsProvisioningString(int subId, int key) throws android.os.RemoteException;
  /**
       * Set the integer provisioning value for the provisioning key specified.
       */
  public int setImsProvisioningInt(int subId, int key, int value) throws android.os.RemoteException;
  /**
       * Set the String provisioning value for the provisioning key specified.
       */
  public int setImsProvisioningString(int subId, int key, java.lang.String value) throws android.os.RemoteException;
  /**
       * Update Emergency Number List for Test Mode.
       */
  public void updateEmergencyNumberListTestMode(int action, android.telephony.emergency.EmergencyNumber num) throws android.os.RemoteException;
  /**
       * Get the full emergency number list for Test Mode.
       */
  public java.util.List<java.lang.String> getEmergencyNumberListTestMode() throws android.os.RemoteException;
  /**
       * Enable or disable a logical modem stack associated with the slotIndex.
       */
  public boolean enableModemForSlot(int slotIndex, boolean enable) throws android.os.RemoteException;
  /**
       * Indicate if the enablement of multi SIM functionality is restricted.
       * @hide
       */
  public void setMultiSimCarrierRestriction(boolean isMultiSimCarrierRestricted) throws android.os.RemoteException;
  /**
       * Returns if the usage of multiple SIM cards at the same time is supported.
       *
       * @param callingPackage The package making the call.
       * @return {@link #MULTISIM_ALLOWED} if the device supports multiple SIMs.
       * {@link #MULTISIM_NOT_SUPPORTED_BY_HARDWARE} if the device does not support multiple SIMs.
       * {@link #MULTISIM_NOT_SUPPORTED_BY_CARRIER} in the device supports multiple SIMs, but the
       * functionality is restricted by the carrier.
       */
  public int isMultiSimSupported(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Switch configs to enable multi-sim or switch back to single-sim
       * @hide
       */
  public void switchMultiSimConfig(int numOfSims) throws android.os.RemoteException;
  /**
       * Get if altering modems configurations will trigger reboot.
       * @hide
       */
  public boolean doesSwitchMultiSimConfigTriggerReboot(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Get the mapping from logical slots to physical slots.
       */
  public int[] getSlotsMapping() throws android.os.RemoteException;
  /**
       * Get the IRadio HAL Version encoded as 100 * MAJOR_VERSION + MINOR_VERSION or -1 if unknown
       */
  public int getRadioHalVersion() throws android.os.RemoteException;
  public boolean isModemEnabledForSlot(int slotIndex, java.lang.String callingPackage) throws android.os.RemoteException;
  public boolean isDataEnabledForApn(int apnType, int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  public boolean isApnMetered(int apnType, int subId) throws android.os.RemoteException;
  /**
       * Enqueue a pending sms Consumer, which will answer with the user specified selection for an
       * outgoing SmsManager operation.
       */
  public void enqueueSmsPickResult(java.lang.String callingPackage, com.android.internal.telephony.IIntegerConsumer subIdResult) throws android.os.RemoteException;
  /**
       * Returns the MMS user agent.
       */
  public java.lang.String getMmsUserAgent(int subId) throws android.os.RemoteException;
  /**
       * Returns the MMS user agent profile URL.
       */
  public java.lang.String getMmsUAProfUrl(int subId) throws android.os.RemoteException;
  /**
       * Set allowing mobile data during voice call.
       */
  public boolean setDataAllowedDuringVoiceCall(int subId, boolean allow) throws android.os.RemoteException;
  /**
       * Check whether data is allowed during voice call. Note this is for dual sim device that
       * data might be disabled on non-default data subscription but explicitly turned on by settings.
       */
  public boolean isDataAllowedInVoiceCall(int subId) throws android.os.RemoteException;
}
