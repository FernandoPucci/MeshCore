#pragma once

#include <stdint.h>

class NodePrefs;
class MyMesh;
class MultiSerialInterface;

struct DiagnosticData {
  bool battery_valid;
  uint16_t battery_millivolts;
  uint8_t battery_percent;

  uint64_t uptime_seconds;

  bool radio_metrics_valid;
  int16_t last_rssi_dbm;
  float last_snr_db;

  uint32_t packets_rx;
  uint32_t packets_tx;
  uint16_t known_contacts;

  bool frequency_valid;
  float frequency_mhz;
  bool tx_power_valid;
  int8_t tx_power_dbm;

  bool ble_enabled;
  bool ble_connected;
};

class DiagnosticDataSource {
  uint32_t _last_millis;
  uint64_t _uptime_millis;
  bool _uptime_started;

public:
  DiagnosticDataSource() : _last_millis(0), _uptime_millis(0), _uptime_started(false) { }

  void begin();
  void tick();
  void capture(DiagnosticData& dest, const NodePrefs& prefs, const MyMesh& mesh,
               MultiSerialInterface& interfaces) const;
};
