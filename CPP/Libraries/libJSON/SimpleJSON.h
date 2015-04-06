#ifndef DEV_TOOLS_SIMPLE_JSON_H__
#define DEV_TOOLS_SIMPLE_JSON_H__
#include <vector>
#include <string>
#include <tuple>
#include <map>

#include <rapidjson/reader.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

/**
 * The simple JSON Builder wraps the rapidjson writer to provide a simpler, but
 * slower interface. It is designed for situations where the JSON write time is
 * note a critical bottle neck, for raw speed rapidjson should be used directly.
 */
class SimpleJSONBuilder {
public: 
    SimpleJSONBuilder();
    void Add(const std::string& name, const std::string& value);

    void Add(const std::string& name,
             const std::vector<std::string>& stringArray);

    void Add(const std::string& name, const int& value);

    void Add(const std::string& name, const int64_t& value);

    void Add(const std::string& name, const unsigned& value);

    void Add(const std::string& name, const uint64_t& value);

    void Add(const std::string& name, const double& value);

    void Add(const std::string& name, const bool& value);

    /*
     * Reset the builder, as if it was a newly constructed object.
     */
    void Clear();

    /**
     * Return the current object as a JSON string, and reset the builder.
     */
    std::string GetAndClear();
private:
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer;
};

/*****************************************************************************
 *                          Field Type Definitions
 *                         (see .hpp file for details)
 *****************************************************************************/

struct FieldBase {
    enum FieldType {
        STRING,
        STRING_ARRAY,
        INT,
        INT_64,
        UINT,
        UINT_64,
        DOUBLE,
        BOOL
    };

    virtual ~FieldBase() { }

    /*
     * Reset the state of this field (as if it were newly created, and not
     * "parsed")
     */
    virtual void Clear() = 0;

    /*
     * Access to the string name of this field
     */
    virtual const char* Name() = 0;

    /**
     * Utility for verifying the type of this field.
     */
    virtual FieldType Type() = 0;

};

/*
 * Macros for adding new field of a particular type.
 *
 * NOTE: The FieldName will need to be unique...
 */
#define NewStringField(FieldName) struct FieldName: public StringField  { const char * Name() { return #FieldName; } };
#define NewIntField(FieldName)    struct FieldName: public IntField     { const char * Name() { return #FieldName; } };
#define NewI64Field(FieldName)    struct FieldName: public I64Field     { const char * Name() { return #FieldName; } };
#define NewUIntField(FieldName)   struct FieldName: public UIntField    { const char * Name() { return #FieldName; } };
#define NewUI64Field(FieldName)   struct FieldName: public UI64Field    { const char * Name() { return #FieldName; } };
#define NewDoubleField(FieldName) struct FieldName: public DoubleField  { const char * Name() { return #FieldName; } };
#define NewBoolField(FieldName)   struct FieldName: public BoolField    { const char * Name() { return #FieldName; } };
#define NewStringArrayField(FieldName)   struct FieldName: public StringArrayField    { const char * Name() { return #FieldName; } };


/**
 * The simple parser takes in a map configuration of fields and their type.
 */
template <class...Fields>
class SimpleParsedJSON {
public:
    /**************************************************************************
     *                      Initialise the Parser
     **************************************************************************/
    /**
     * C'tor
     *
     * Initialise the run-time map which will convert field name into the actual
     * field.
     */
    SimpleParsedJSON();

    /**
     * Reset the object, as if it was newly constructed and ready to parse a new
     * JSON object.
     *
     * This is much cheaper than construction since we don't have to re-build
     * the map.
     */
    void Clear();

    /**************************************************************************
     *                    Run the Parser
     **************************************************************************/

    /**
     * Parse the json string provided and store the fields in our members. Raise
     * an error if unknown fields are provided, or if known fields are of the
     * wrong type. Finally raise an error if a field is not provided.
     *
     * In the case of an error Parse returns false, and errMsg will be populated
     * appropriately
     *
     * WARNING: If this is not the first call to Parse, Clear() should be called
     *          first
     *
     * @param json     The JSON to parse
     * @param errMsg   Will be populated with an error if the function returns
     *                 false
     *
     * @returns TRUE if all (and only) our fields were found in the JSON
     */
    bool Parse(const char* json, std::string& errMsg);

    /**************************************************************************
     *                    Access Results
     **************************************************************************/

    /**
     * Get a reference to the value of a particular field
     *
     * @returns Lookup the field
     */
    template <class FIELD>
    typename FIELD::ValueType& Get();

    /**
     * Get a JSON string which represents the parsed JSON
     *
     * @returns A valid JSON string
     */
    std::string GetJSONString();

    /**************************************************************************
     *                      Rapid JSON Implementation
     *                     (see .hpp file for details)
     **************************************************************************/

    bool Key(const char* str, rapidjson::SizeType length, bool copy);

    bool String(const char* str, rapidjson::SizeType length, bool copy);

    bool Int(int i);

    bool Uint(unsigned u);

    bool Double(double d);

    bool StartObject();

    bool EndObject(rapidjson::SizeType memberCount);

    bool Null();

    bool Bool(bool b);

    bool Int64(int64_t i);

    bool Uint64(uint64_t u);

    bool StartArray();

    bool EndArray(rapidjson::SizeType elementCount);

private:
    /**************************************************************************
     *                      Internal Errors
     **************************************************************************/

    struct UnknownFieldError {
        std::string field;
    };

    struct WrongTypeError {
        std::string field;
    };

    struct ValueError {
        std::string field;
    };

    class ParseError { };

    class UnknownTypeError { };

    /**************************************************************************
     *                      Internal Utilities
     **************************************************************************/

    struct FieldInfo {
        FieldBase* const field;
        const FieldBase::FieldType type;
        const char* const name;
    };

    FieldInfo* Get(const char* fieldName);

    /**************************************************************************
     *                      Runtime initialisation
     *   (Loop over each field with AddNextField, calling AddField for each)
     **************************************************************************/

    template <int idx>
    inline typename std::enable_if<idx!=0,void>::type
    AddNextField() {
        AddField<idx-1>();
        AddNextField<idx-1>();
    }

    template <int idx>
    inline typename std::enable_if<idx==0,void>::type
    AddNextField() { }

    /**
     * Lookup the field with index idx, and store its name in the map with a
     * reference to the fields location within the tuple.
     *
     * This will be used at parse time to update the correct field based on its
     * name.
     */
    template <int idx>
    void AddField();

    /**************************************************************************
     *           Convert each field to its JSON representation
     *  (Loop over each field with PrintNextField, calling PrintField for each)
     **************************************************************************/

    template <int idx>
    inline typename std::enable_if<idx!=0,void>::type
    PrintNextField() {
        PrintField<idx-1>();
        PrintNextField<idx-1>();
    }

    template <int idx>
    inline typename std::enable_if<idx==0,void>::type
    PrintNextField() { }

    /**
     * Lookup the field with index idx, and store its name in the map with a
     * reference to the fields location within the tuple.
     *
     * This will be used at parse time to update the correct field based on its
     * name.
     */
    template <int idx>
    void PrintField();

    /**************************************************************************
     *                      Internal Data
     **************************************************************************/

    // The field currently being passes
    FieldInfo* currentField;

    // Our complete set of fields
    std::tuple<Fields...> fields;

    // The map from field name to the its location within the tuple.
    typedef std::map<std::string,FieldInfo> FieldMap;
    FieldMap fieldMap;

    // Tracks if we are in a sub-object (not current supported!)
    size_t depth;

    // Tracks if we are currently handling an array...
    bool isArray;

    // Used to build a string representation of the JSON
    SimpleJSONBuilder builder;
};


#include "SimpleJSON.hpp"

#endif