bool deletePreset(uint8_t index) {
  // Check if the index is valid
  if (index >= flashData.presetCount) {
    Serial.println("Invalid preset index.");
    return false;
  }

  // Start for loop at index and shift subsequnt presets one to the front
  for (uint8_t i = index; i < flashData.presetCount - 1; i++) {
    flashData.presets[i] = flashData.presets[i + 1];
  }

  flashData.presetCount--;
  totalPresetSelectOptions--;

  memset(&flashData.presets[flashData.presetCount], 0, sizeof(PresetData));

  writeFlashData();

  Serial.println("Preset deleted and presets shifted successfully.");
  return true;
}
