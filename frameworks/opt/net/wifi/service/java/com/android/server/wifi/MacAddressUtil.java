/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.server.wifi;

import android.net.MacAddress;
import android.net.wifi.WifiConfiguration;
import android.security.keystore.AndroidKeyStoreProvider;
import android.security.keystore.KeyGenParameterSpec;
import android.security.keystore.KeyProperties;
import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.InvalidAlgorithmParameterException;
import java.security.InvalidKeyException;
import java.security.Key;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.ProviderException;
import java.security.UnrecoverableKeyException;
import java.util.Arrays;

import javax.crypto.KeyGenerator;
import javax.crypto.Mac;
import javax.crypto.SecretKey;

/**
 * Contains helper methods to support MAC randomization.
 */
public class MacAddressUtil {
    private static final String TAG = "MacAddressUtil";
    private static final String MAC_RANDOMIZATION_ALIAS = "MacRandSecret";
    private static final long MAC_ADDRESS_VALID_LONG_MASK = (1L << 48) - 1;
    private static final long MAC_ADDRESS_LOCALLY_ASSIGNED_MASK = 1L << 41;
    private static final long MAC_ADDRESS_MULTICAST_MASK = 1L << 40;

    /**
     * Computes the persistent randomized MAC of the given configuration using the given
     * hash function.
     * @param config the WifiConfiguration to compute MAC address for
     * @param hashFunction the hash function that will perform the MAC address computation.
     * @return The persistent randomized MAC address or null if inputs are invalid.
     */
    public MacAddress calculatePersistentMacForConfiguration(WifiConfiguration config,
            Mac hashFunction) {
        if (config == null || hashFunction == null) {
            return null;
        }
        byte[] hashedBytes;
        try {
            hashedBytes = hashFunction.doFinal(config.getSsidAndSecurityTypeString()
                    .getBytes(StandardCharsets.UTF_8));
        } catch (ProviderException | IllegalStateException e) {
            Log.e(TAG, "Failure in calculatePersistentMac", e);
            return null;
        }
        ByteBuffer bf = ByteBuffer.wrap(hashedBytes);
        long longFromSsid = bf.getLong();
        /**
         * Masks the generated long so that it represents a valid randomized MAC address.
         * Specifically, this sets the locally assigned bit to 1, multicast bit to 0
         */
        longFromSsid &= MAC_ADDRESS_VALID_LONG_MASK;
        longFromSsid |= MAC_ADDRESS_LOCALLY_ASSIGNED_MASK;
        longFromSsid &= ~MAC_ADDRESS_MULTICAST_MASK;
        bf.clear();
        bf.putLong(0, longFromSsid);

        // MacAddress.fromBytes requires input of length 6, which is obtained from the
        // last 6 bytes from the generated long.
        MacAddress macAddress = MacAddress.fromBytes(Arrays.copyOfRange(bf.array(), 2, 8));
        return macAddress;
    }

    /**
     * Retrieves a Hash function that could be used to calculate the persistent randomized MAC
     * for a WifiConfiguration.
     * @param uid the UID of the KeyStore to get the secret of the hash function from.
     */
    public Mac obtainMacRandHashFunction(int uid) {
        try {
            KeyStore keyStore = AndroidKeyStoreProvider.getKeyStoreForUid(uid);
            // tries to retrieve the secret, and generate a new one if it's unavailable.
            Key key = keyStore.getKey(MAC_RANDOMIZATION_ALIAS, null);
            if (key == null) {
                key = generateAndPersistNewMacRandomizationSecret(uid);
            }
            if (key == null) {
                Log.e(TAG, "Failed to generate secret for " + MAC_RANDOMIZATION_ALIAS);
                return null;
            }
            Mac result = Mac.getInstance("HmacSHA256");
            result.init(key);
            return result;
        } catch (KeyStoreException | NoSuchAlgorithmException | InvalidKeyException
                | UnrecoverableKeyException | NoSuchProviderException e) {
            Log.e(TAG, "Failure in obtainMacRandHashFunction", e);
            return null;
        }
    }

    /**
     * Generates and returns a secret key to use for Mac randomization.
     * Will also persist the generated secret inside KeyStore, accessible in the
     * future with KeyGenerator#getKey.
     */
    private SecretKey generateAndPersistNewMacRandomizationSecret(int uid) {
        try {
            KeyGenerator keyGenerator = KeyGenerator.getInstance(
                    KeyProperties.KEY_ALGORITHM_HMAC_SHA256, "AndroidKeyStore");
            keyGenerator.init(
                    new KeyGenParameterSpec.Builder(MAC_RANDOMIZATION_ALIAS,
                            KeyProperties.PURPOSE_SIGN)
                            .setUid(uid)
                            .build());
            return keyGenerator.generateKey();
        } catch (NoSuchAlgorithmException | InvalidAlgorithmParameterException
                | NoSuchProviderException | ProviderException e) {
            Log.e(TAG, "Failure in generateMacRandomizationSecret", e);
            return null;
        }
    }
}
