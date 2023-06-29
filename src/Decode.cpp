#include <sstream>

#include <avro/ValidSchema.hh>
#include <avro/GenericDatum.hh>
#include <avro/Generic.hh>
#include <avro/Decoder.hh>
#include <avro/Stream.hh>

#include "HelperFunctions.h"
#include "Schema.h"
#include "Decode.h"
#include "TypeCheck.h"


K decodeMap(const avro::GenericMap& avro_map);
K decodeRecord(const avro::GenericRecord& record);
K decodeUnion(const avro::GenericDatum& avro_union);

K decodeArray(const avro::GenericArray& array_datum)
{
  auto array_schema = array_datum.schema();
  assert(array_schema->leaves() == 1);
  auto array_type = array_schema->leafAt(0)->type();
  const auto& array_data = array_datum.value();

  size_t result_len = array_data.size();
  if (array_type == avro::AVRO_RECORD || array_type == avro::AVRO_MAP) {
    // We put a (::) at the start of an array of records/maps so need one more item
    ++result_len;
  }
  K result = ktn(GetKdbArrayType(array_type), result_len);
  size_t index = 0;

  switch (array_type) {
  case avro::AVRO_BOOL:
  {
    for (auto i : array_data)
      kG(result)[index++] = i.value<bool>();
    break;
  }
  case avro::AVRO_BYTES:
  {
    for (auto i : array_data) {
      const auto& bytes = i.value<std::vector<uint8_t>>();
      K k_bytes = ktn(KG, bytes.size());
      std::memcpy(kG(k_bytes), bytes.data(), bytes.size());
      kK(result)[index++] = k_bytes;
    }
    break;
  }
  case avro::AVRO_DOUBLE:
  {
    for (auto i : array_data)
      kF(result)[index++] = i.value<double>();
    break;
  }
  case avro::AVRO_ENUM:
  {
    for (auto i : array_data)
      kS(result)[index++] = ss((S)i.value<avro::GenericEnum>().symbol().c_str());
    break;
  }
  case avro::AVRO_FIXED:
  {
    for (auto i : array_data) {
      const auto& fixed = i.value<avro::GenericFixed>().value();
      K k_fixed = ktn(KG, fixed.size());
      std::memcpy(kG(k_fixed), fixed.data(), fixed.size());
      kK(result)[index++] = k_fixed;
    }
    break;
  }
  case avro::AVRO_FLOAT:
  {
    for (auto i : array_data)
      kE(result)[index++] = i.value<float>();
    break;
  }
  case avro::AVRO_INT:
  {
    for (auto i : array_data)
      kI(result)[index++] = i.value<int32_t>();
    break;
  }
  case avro::AVRO_LONG:
  {
    for (auto i : array_data)
      kJ(result)[index++] = i.value<int64_t>();
    break;
  }
  case avro::AVRO_NULL:
  {
    for (auto i : array_data)
      kK(result)[index++] = identity();
    break;
  }
  case avro::AVRO_STRING:
  {
    for (auto i : array_data) {
      const auto& string = i.value<std::string>();
      K k_string = ktn(KC, string.length());
      std::memcpy(kG(k_string), string.c_str(), string.length());
      kK(result)[index++] = k_string;
    }
    break;
  }
  case avro::AVRO_RECORD:
  {
    kK(result)[index++] = identity();
    for (auto i : array_data)
      kK(result)[index++] = decodeRecord(i.value<avro::GenericRecord>());
    break;
  }
  case avro::AVRO_ARRAY:
  {
    for (auto i : array_data)
      kK(result)[index++] = decodeArray(i.value<avro::GenericArray>());
    break;
  }
  case avro::AVRO_UNION:
  {
    for (auto i : array_data)
      kK(result)[index++] = decodeUnion(i);
    break;
  }
  case avro::AVRO_MAP:
  {
    kK(result)[index++] = identity();
    for (auto i : array_data)
      kK(result)[index++] = decodeMap(i.value<avro::GenericMap>());
    break;
  }

  case avro::AVRO_SYMBOLIC:
  case avro::AVRO_UNKNOWN:
  default:
    throw TypeCheck("Unsupported type");
  }

  return result;
}

K decodeMap(const avro::GenericMap& map_datum)
{
  auto map_schema = map_datum.schema();
  assert(map_schema->leaves() == 2);
  auto map_type = map_schema->leafAt(1)->type();
  const auto& map_data = map_datum.value();

  size_t result_len = map_data.size();
  if (map_type == avro::AVRO_RECORD || map_type == avro::AVRO_MAP) {
    // We put a (::) at the start of an array of records/maps so need one more item
    ++result_len;
  }

  K keys = ktn(KS, result_len);
  K values = ktn(GetKdbArrayType(map_type), result_len);
  size_t index = 0;

  switch (map_type) {
  case avro::AVRO_BOOL:
  {
    for (auto i : map_data) {
      kS(keys)[index] = ss((S)i.first.c_str());
      kG(values)[index++] = i.second.value<bool>();
    }
    break;
  }
  case avro::AVRO_BYTES:
  {
    for (auto i : map_data) {
      kS(keys)[index] = ss((S)i.first.c_str());
      const auto& bytes = i.second.value<std::vector<uint8_t>>();
      K k_bytes = ktn(KG, bytes.size());
      std::memcpy(kG(k_bytes), bytes.data(), bytes.size());
      kK(values)[index++] = k_bytes;
    }
    break;
  }
  case avro::AVRO_DOUBLE:
  {
    for (auto i : map_data) {
      kS(keys)[index] = ss((S)i.first.c_str());
      kF(values)[index++] = i.second.value<double>();
    }
    break;
  }
  case avro::AVRO_ENUM:
  {
    for (auto i : map_data) {
      kS(keys)[index] = ss((S)i.first.c_str());
      kS(values)[index++] = ss((S)i.second.value<avro::GenericEnum>().symbol().c_str());
    }
    break;
  }
  case avro::AVRO_FIXED:
  {
    for (auto i : map_data) {
      kS(keys)[index] = ss((S)i.first.c_str());
      const auto& fixed = i.second.value<avro::GenericFixed>().value();
      K k_fixed = ktn(KG, fixed.size());
      std::memcpy(kG(k_fixed), fixed.data(), fixed.size());
      kK(values)[index++] = k_fixed;
    }
    break;
  }
  case avro::AVRO_FLOAT:
  {
    for (auto i : map_data) {
      kS(keys)[index] = ss((S)i.first.c_str());
      kE(values)[index++] = i.second.value<float>();
    }
    break;
  }
  case avro::AVRO_INT:
  {
    for (auto i : map_data) {
      kS(keys)[index] = ss((S)i.first.c_str());
      kI(values)[index++] = i.second.value<int32_t>();
    }
    break;
  }
  case avro::AVRO_LONG:
  {
    for (auto i : map_data) {
      kS(keys)[index] = ss((S)i.first.c_str());
      kJ(values)[index++] = i.second.value<int64_t>();
    }
    break;
  }
  case avro::AVRO_NULL:
  {
    for (auto i : map_data) {
      kS(keys)[index] = ss((S)i.first.c_str());
      kK(values)[index++] = identity();
    }
    break;
  }
  case avro::AVRO_STRING:
  {
    for (auto i : map_data) {
      kS(keys)[index] = ss((S)i.first.c_str());
      const auto& string = i.second.value<std::string>();
      K k_string = ktn(KC, string.length());
      std::memcpy(kG(k_string), string.c_str(), string.length());
      kK(values)[index++] = k_string;
    }
    break;
  }
  case avro::AVRO_RECORD:
  {
    kS(values)[index] = ss((S)"");
    kK(values)[index++] = identity();
    for (auto i : map_data) {
      kS(keys)[index] = ss((S)i.first.c_str());
      kK(values)[index++] = decodeRecord(i.second.value<avro::GenericRecord>());
    }
    break;
  }
  case avro::AVRO_ARRAY:
  {
    for (auto i : map_data) {
      kS(keys)[index] = ss((S)i.first.c_str());
      kK(values)[index++] = decodeArray(i.second.value<avro::GenericArray>());
    }
    break;
  }
  case avro::AVRO_UNION:
  {
    for (auto i : map_data) {
      kS(keys)[index] = ss((S)i.first.c_str());
      kK(values)[index++] = decodeUnion(i.second);
    }
    break;
  }
  case avro::AVRO_MAP:
  {
    kS(keys)[index] = ss((S)"");
    kK(values)[index++] = identity();
    for (auto i : map_data) {
      kS(keys)[index] = ss((S)i.first.c_str());
      kK(values)[index++] = decodeMap(i.second.value<avro::GenericMap>());
    }
    break;
  }

  case avro::AVRO_SYMBOLIC:
  case avro::AVRO_UNKNOWN:
  default:
    throw TypeCheck("Unsupported type");
  }

  return xD(keys, values);
}

K decodeDatum(const avro::GenericDatum& datum, bool decompose_union)
{
  avro::Type avro_type;
  if (!decompose_union)
    avro_type = GetRealType(datum);
  else
    avro_type = datum.type();

  switch (avro_type) {
  case avro::AVRO_BOOL:
    return kb(datum.value<bool>());
  case avro::AVRO_BYTES:
  {
    const auto& bytes = datum.value<std::vector<uint8_t>>();
    K result = ktn(KG, bytes.size());
    std::memcpy(kG(result), bytes.data(), bytes.size());
    return result;
  }
  case avro::AVRO_DOUBLE:
    return kf(datum.value<double>());
  case avro::AVRO_ENUM:
    return ks(S(datum.value<avro::GenericEnum>().symbol().c_str()));
  case avro::AVRO_FIXED:
  {
    const auto & fixed = datum.value<avro::GenericFixed>().value();
    K result = ktn(KG, fixed.size());
    std::memcpy(kG(result), fixed.data(), fixed.size());
    return result;
  }
  case avro::AVRO_FLOAT:
    return ke(datum.value<float>());
  case avro::AVRO_INT:
    return ki(datum.value<int32_t>());
  case avro::AVRO_LONG:
    return kj(datum.value<int64_t>());
  case avro::AVRO_NULL:
    return identity();;
  case avro::AVRO_STRING:
  {
    const auto& string = datum.value<std::string>();
    K result = ktn(KC, string.length());
    std::memcpy(kG(result), string.c_str(), string.length());
    return result;
  }
  case avro::AVRO_RECORD:
    return decodeRecord(datum.value<avro::GenericRecord>());
  case avro::AVRO_ARRAY:
    return decodeArray(datum.value<avro::GenericArray>());
  case avro::AVRO_UNION:
    return decodeUnion(datum);
  case avro::AVRO_MAP:
    return decodeMap(datum.value<avro::GenericMap>());

  case avro::AVRO_SYMBOLIC:
  case avro::AVRO_UNKNOWN:
  default:
    throw TypeCheck("Unsupported type");
  }
}

K decodeRecord(const avro::GenericRecord& record)
{
  K keys = ktn(KS, record.fieldCount() + 1);
  kS(keys)[0] = ss((S)"");
  K values = ktn(0, record.fieldCount() + 1);
  kK(values)[0] = identity();

  size_t index = 1;
  for (auto i = 0; i < record.fieldCount(); ++i) {
    const auto& next = record.fieldAt(i);
    const auto& name = record.schema()->nameAt(i);
    kS(keys)[index] = ss((S)name.c_str());
    kK(values)[index] = decodeDatum(next, false);
    ++index;
  }

  return xD(keys, values);
}

K decodeUnion(const avro::GenericDatum& avro_union)
{
  K result = ktn(0, 2);
  kK(result)[0] = kh((I)avro_union.unionBranch());
  kK(result)[1] = decodeDatum(avro_union, true);

  return result;
}

K decode(K schema, K data)
{
  if (data->t != KG)
    return krr((S)"data not 4h");

  KDB_EXCEPTION_TRY;

  auto avro_schema = GetAvroSchema(schema);

  auto decoder = avro::validatingDecoder(*avro_schema.get(), avro::binaryDecoder());

  std::istringstream iss;
  iss.str(std::string((char*)kG(data), data->n));

  auto istream = avro::istreamInputStream(iss);
  decoder->init(*istream);

  avro::GenericReader reader(*avro_schema.get(), decoder);

  avro::GenericDatum datum;
  reader.read(datum);
  reader.drain();

  return decodeDatum(datum, false);

  KDB_EXCEPTION_CATCH;
}
