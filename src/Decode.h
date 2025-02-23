#pragma once

extern "C"
{
  /// @brief Decode Avro serialised data to a kdb+ object
  ///
  /// Supported options:
  ///
  /// * AVRO_FORMAT (string).  Describes whether the Avro serialised data is in
  /// binary or JSON format.  Valid options "BINARY" or "JSON", default
  /// "BINARY".
  ///
  /// @param schema.  Foreign object containing the Avro schema to use for
  /// decoding. 
  ///
  /// @param data.  4h or 10h list of Avro serialised data.
  ///
  /// @param options. kdb+ dictionary of options or generic null(::) to use the
  /// defaults.  Dictionary key must be a 11h list.  Values list can be 7h, 11h
  /// or mixed list of -7|-11|4h.
  ///
  /// @return kdb+ object representing the Avro data having applied the
  /// appropriate type mappings
  EXP K Decode(K schema, K data, K options);
}
