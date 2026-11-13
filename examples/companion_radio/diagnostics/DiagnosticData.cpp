#include "DiagnosticData.h"

#include "../MyMesh.h"
#include "../NodePrefs.h"
#include <Arduino.h>
#include <helpers/MultiSerialInterface.h>
#include <target.h>

#ifndef BATT_MIN_MILLIVOLTS
  #define BATT_MIN_MILLIVOLTS 3000
#endif
#ifndef BATT_MAX_MILLIVOLTS
  #define BATT_MAX_MILLIVOLTS 4200
#endif

static uint8_t batteryPercent(uint16_t millivolts) {
  if (millivolts <= BATT_MIN_MILLIVOLTS) return 0;
  if (millivolts >= BATT_MAX_MILLIVOLTS) return 100;

  return (uint8_t)(((uint32_t)(millivolts - BATT_MIN_MILLIVOLTS) * 100) /
                   (BATT_MAX_MILLIVOLTS - BATT_MIN_MILLIVOLTS));
}

void DiagnosticDataSource::begin() {
  _last_millis = millis();
  _uptime_millis = 0;
  _uptime_started = true;
}

void DiagnosticDataSource::tick() {
  uint32_t now = millis();
  if (!_uptime_started) {
    _last_millis = now;
    _uptime_started = true;
    return;
  }

  // Unsigned subtraction remains correct when millis() rolls over.
  _uptime_millis += (uint32_t)(now - _last_millis);
  _last_millis = now;
}

void DiagnosticDataSource::capture(DiagnosticData& dest, const NodePrefs& prefs, const MyMesh& mesh,
                                   MultiSerialInterface& interfaces) const {
  dest = {};

  dest.battery_millivolts = board.getBattMilliVolts();
  dest.battery_valid = dest.battery_millivolts > 0;
  if (dest.battery_valid) {
    dest.battery_percent = batteryPercent(dest.battery_millivolts);
  }

  dest.uptime_seconds = _uptime_millis / 1000;

  dest.packets_rx = radio_driver.getPacketsRecv();
  dest.packets_tx = radio_driver.getPacketsSent();
  dest.radio_metrics_valid = dest.packets_rx > 0;
  if (dest.radio_metrics_valid) {
    dest.last_rssi_dbm = (int16_t)radio_driver.getLastRSSI();
    dest.last_snr_db = radio_driver.getLastSNR();
  }

  int contacts = mesh.getNumContacts();
  dest.known_contacts = contacts > 0 ? (uint16_t)contacts : 0;

  dest.frequency_mhz = prefs.freq;
  dest.frequency_valid = prefs.freq > 0;
  dest.tx_power_dbm = prefs.tx_power_dbm;
  dest.tx_power_valid = true;

  dest.ble_enabled = interfaces.isBluetoothEnabled();
  dest.ble_connected = interfaces.isBluetoothConnected();
}
