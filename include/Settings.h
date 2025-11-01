#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

/**
 * Settings - Clean wrapper around ESP32 Preferences
 * 
 * Provides a simple key-value storage API for persistent configuration.
 * Uses ESP32's NVS (Non-Volatile Storage) under the hood.
 * 
 * Based on the pattern from e-paper project.
 */

// Initialize the settings system
void settings_init();

// Check if a key exists
bool settings_has_key(const char* key);

// String operations
String settings_get_string(const char* key, const char* default_value = "");
void settings_put_string(const char* key, const char* value);

// Integer operations
int settings_get_int(const char* key, int default_value = 0);
void settings_put_int(const char* key, int value);

// Boolean operations
bool settings_get_bool(const char* key, bool default_value = false);
void settings_put_bool(const char* key, bool value);

// Float operations
float settings_get_float(const char* key, float default_value = 0.0f);
void settings_put_float(const char* key, float value);

// Factory reset - clear all settings
void settings_clear();

#endif // SETTINGS_H
