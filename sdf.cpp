#include "sdf.hpp"

// for convenience
using json = nlohmann::json;
using namespace std;


map<string, sdfCommon*> existingDefinitons;

string jsonDTypeToString(jsonDataType type)
{
    switch (type)
    {
    case json_number:
        return "number";
        break;
    case json_string:
        return "string";
        break;
    case json_boolean:
        return "boolean";
        break;
    case json_integer:
        return "integer";
        break;
    case json_array:
        return "array";
        break;
    case json_object:
        return "object";
        break;
    case json_type_undef:
        return "";
        break;
    default:
        cerr << "jsonDTypeToString(): parameter is invalid type" << endl;
        return "";
        break;
    }
}

jsonDataType stringToJsonDType(string str)
{
    if (str == "number" || str == "decimal64")
        return json_number;
    else if (str == "string")
        return json_string;
    else if (str == "boolean" || str == "bool")
        return json_boolean;
    else if (str == "integer" || str == "int"
            || str == "int8" || str == "uint8"
            || str == "int16" || str == "unint16"
            || str == "int32" || str == "uint32"
            || str == "int64" || str == "unint64")
        return json_integer;
    else if (str == "array")
        return json_array;
    else if (str == "object")
        return json_object;

    return json_type_undef;
}

sdfCommon* refToCommon(string ref)
{
    /*
    smatch sm;
    regex split_regex("#(/(.*))+");
    regex_match(ref, sm, split_regex);
    cout << sm[2] << endl;
    */
    regex sameNsRegex("#/.*");
    if (!regex_match(ref, sameNsRegex))
    {
        cerr << "Definition from different namespace could not be loaded"
                << endl;
        return NULL;
    }
    else if (existingDefinitons[ref] != NULL)
    {
        //cout<<"refToCommon: "<<existingDefinitons[ref]->getLabel()<<endl;
        return existingDefinitons[ref];
    }
    else
    {
        cerr << "refToCommon(): definition for reference "
                << ref << " does not exist" << endl;
        return NULL;
    }
}

/*
 * Necessary because iterator.value() replaces _ with space (???)
 */
string correctValue(string val)
{
    string correct = val;
    for(int i = 0; i < correct.length(); i++)
    {
       if(isspace(correct[i]))
           correct[i] = '_';
    }
    return correct;
}

sdfCommon::sdfCommon(string _label, string _description, sdfCommon *_reference,
        vector<sdfCommon*> _required)
            : description(_description), label(_label), reference(_reference),
              required(_required)
{
    //this->parent = NULL;
}

sdfCommon::~sdfCommon()
{
    delete[] required.data();
    delete reference;
}

void sdfCommon::setLabel(string _label)
{
    this->label = _label;
}

string sdfCommon::getDescription()
{
    return description;
}

string sdfCommon::getLabel()
{
    return label;
}

const char* sdfCommon::getLabelAsArray()
{
    return label.c_str();
}

sdfCommon* sdfCommon::getReference()
{
    return this->reference;
}

/*
string sdfCommon::getReferenceAsString()
{
    // TODO: for the reference (not the common!) create a string
    // (e.g. "#/sdfObject/temperatureWithAlarm/sdfData/temperatureData")
    // or return empty string if there is no reference (try catch block?)
    return "";
}
*/
vector<sdfCommon*> sdfCommon::getRequired()
{
    return required;
}
/*
string sdfCommon::getPointer()
{

    return this->jsonPointer.to_string();
}
*/

void sdfCommon::addRequired(sdfCommon *common)
{
    this->required.push_back(common);
}

void sdfCommon::setReference(sdfCommon *common)
{
    this->reference = common;
}


json sdfCommon::commonToJson(json prefix)
{
    if (this->getReference() != NULL)
    {
        prefix["sdfRef"] = this->getReference()->generateReferenceString();
        //cout << this->reference->generateReferenceString()<<endl;
    }
    if (this->getLabel() != "")
        prefix["label"] = this->getLabel();
    if (this->getDescription() != "")
        prefix["description"] = this->getDescription();
    vector<string> req = {};
    for (sdfCommon *i : this->required)
    {
        req.push_back(i->generateReferenceString());
    }
    if (!req.empty())
        prefix["sdfRequired"] = req;

    return prefix;
}

sdfObjectElement::sdfObjectElement(string _label, string _description,
        sdfCommon *_reference, vector<sdfCommon*> _required,
        sdfObject *_parentObject)
            : sdfCommon(_label, _description, _reference, _required),
              parentObject(_parentObject)
{
}

sdfObjectElement::~sdfObjectElement()
{
    delete parentObject;
}

sdfObject* sdfObjectElement::getParentObject()
{
    return parentObject;
}

void sdfObjectElement::setParentObject(sdfObject *_parentObject)
{
    this->parentObject = _parentObject;
}

string sdfObjectElement::generateReferenceString()
{
    if (this->getParentObject() == NULL)
    {
        cerr << "sdfObjectElement::generateReferenceString(): "
                << this->getLabel() + " has no assigned parent object."
                << endl;
        return "";
    }
    else
        return this->parentObject->generateReferenceString() + "/";
            //+ this->getLabel();
}

sdfInfoBlock::sdfInfoBlock(string _title, string _version, string _copyright,
        string _license)
            : title(_title), version(_version), copyright(_copyright),
              license(_license)
{}

string sdfInfoBlock::getTitle()
{
    return title;
}

string sdfInfoBlock::getVersion()
{
    return version;
}

string sdfInfoBlock::getCopyright()
{
    return copyright;
}

string sdfInfoBlock::getLicense()
{
    return license;
}

json sdfInfoBlock::infoToJson(json prefix)
{
    //cout << "info to json" << endl;
    if (this->getTitle() != "")
        prefix["info"]["title"] = this->getTitle();
    if (this->getVersion() != "")
        prefix["info"]["version"] = this->getVersion();
    if (this->getCopyright() != "")
        prefix["info"]["copyright"] = this->getCopyright();
    if (this->getLicense() != "")
        prefix["info"]["license"] = this->getLicense();
    return prefix;
}

sdfNamespaceSection::sdfNamespaceSection(map<string, string> _namespaces,
        string _default_ns)
            : namespaces(_namespaces), default_ns(_default_ns)
{}


map<string, string> sdfNamespaceSection::getNamespaces()
{
    return namespaces;
}

string sdfNamespaceSection::getDefaultNamespace()
{
    return default_ns;
}

json sdfNamespaceSection::namespaceToJson(json prefix)
{
    //cout << "ns to json" << endl;
    for (auto it : this->namespaces)
    {
        prefix["namespace"][it.first] = it.second;
    }
    if (this->default_ns != "")
        prefix["defaultNamespace"] = this->default_ns;
    return prefix;
}

sdfData::sdfData() : sdfData("", "", "")
{}

sdfData::sdfData(string _label, string _description, string _type,
        sdfCommon *_reference, vector<sdfCommon*> _required,
        sdfCommon *_parentCommon, vector<sdfData*> _choice)
            : sdfCommon(_label, _description, _reference, _required),
              parent(_parentCommon)
{
    simpleType = stringToJsonDType(_type);
    if (simpleType == json_type_undef)
        derType = _type;
    else
        derType = "";
    this->setParentCommon(_parentCommon);

    readable = NAN;
    writable = NAN;
    observable = NAN;
    nullable = NAN;
    readableDefined = false;
    writableDefined = false;
    observableDefined = false;
    nullableDefined = false;
    subtype = sdf_subtype_undef;
    format = json_format_undef;
    constDefined = false;
    defaultDefined = false;
    constBoolDefined = false;
    defaultBoolDefined = false;
    constIntDefined = false;
    defaultIntDefined = false;
    constantBool = false;
    defaultBool = false;
    constantNumber = NAN;
    defaultNumber = NAN;
    constantInt = -1;
    defaultInt = -1;
    constantAsCharArray = NULL;
    defaultAsCharArray = NULL;
    exclusiveMaximum_bool = false;
    exclusiveMinimum_bool = false;
    minLength = NAN;
    maxLength = NAN;
    minimum = NAN;
    maximum = NAN;
    multipleOf = NAN;
    exclusiveMinimum_number = NAN;
    exclusiveMaximum_number = NAN;
    pattern = "";
    format = json_format_undef;
    maxItems = NAN;
    minItems = NAN;
    uniqueItems = NAN;
    uniqueItemsDefined = false;
    item_constr = NULL;
    units = "";
    scaleMinimum = NAN;
    scaleMaximum = NAN;
    contentFormat = "";
    sdfChoice = _choice;
    objectProperties = {};
    requiredObjectProperties = {};
}

sdfData::sdfData(string _label, string _description, jsonDataType _type,
        sdfCommon *_reference, vector<sdfCommon*> _required,
        sdfCommon *_parentCommon, vector<sdfData*> _choice)
        : sdfData(_label, _description, jsonDTypeToString(_type), _reference,
        _required, _parentCommon, _choice)
{}

sdfData::~sdfData()
{
    delete[] objectProperties.data();
    delete[] sdfChoice.data();

    delete item_constr;
    /*
    for (sdfData *i : sdfChoice)
    {
        delete i;
    }*/
}

void sdfData::setNumberData(float _constant,
        float _default, float _min, float _max, float _multipleOf)
{
    if (this->simpleType != json_number && simpleType != json_type_undef)
    {
        cerr << this->getLabel() + " cannot be instantiated as type number"
                + " because it already has a different type. "
                + "Please use setSimpType() first to reset it." << endl;
        return;
    }
    if (_min > _max)
        cerr << "Selected minimum value is greater than selected "
                "maximum value: " << _min << " > " << _max << endl;
    this->setSimpType(json_number);
    //enumNumber = _enum;
    constantNumber = _constant;
    defaultNumber = _default;
    minimum = _min;
    maximum = _max;
    multipleOf = _multipleOf;
    if(isnan(_constant))
        constDefined = false;
    else
        constDefined = true;
    if (isnan(_default))
        defaultDefined = false;
    else
        defaultDefined = true;
}

void sdfData::setStringData(string _constant, string _default,
        float _minLength, float _maxLength, string _pattern,
        jsonSchemaFormat _format, vector<string> _enum)
{
    if (simpleType != json_string && simpleType != json_type_undef)
    {
        cerr << this->getLabel() + " cannot be instantiated as type string"
                + " because it already has a different type. "
                + "Please use setSimpType() first to reset it." << endl;
        return;
    }
    this->setSimpType(json_string);
    enumString = _enum;
    constantString = _constant;
    defaultString = _default;
    minLength = _minLength;
    maxLength = _maxLength;
    pattern = _pattern;
    format = _format;
    if(_constant == "")
        constDefined = false;
    else
        constDefined = true;
    if (_default == "")
        defaultDefined = false;
    else
        defaultDefined = true;
}

void sdfData::setBoolData(bool _constant, bool defineConst, bool _default,
        bool defineDefault)
{
    if (simpleType != json_boolean && simpleType != json_type_undef)
    {
        cerr << this->getLabel() + " cannot be instantiated as type boolean"
                + " because it already has a different type. "
                + "Please use setSimpType() first to reset it." << endl;
        return;
    }
    this->setSimpType(json_boolean);
    //enumBool = _enum;
    constDefined = defineConst;
    constBoolDefined = defineConst;
    constantBool = _constant;
    defaultDefined = defineDefault;
    defaultBoolDefined = defineDefault;
    defaultBool = _default;
}

void sdfData::setIntData(int _constant, bool defineConst, int _default,
        bool defineDefault, float _min, float _max)
{
    if (simpleType != json_integer && simpleType != json_type_undef)
    {
        cerr << this->getLabel() + " cannot be instantiated as type integer"
                + " because it already has a different type. "
                + "Please use setSimpType() first to reset it." << endl;
        return;
    }
    if (_min > _max)
        cerr << "Selected minimum value is greater than selected "
                "maximum value: " << _min << " > " << _max << endl;
    this->setSimpType(json_integer);
    //enumInt = _enum;
    constDefined = defineConst;
    constIntDefined = defineConst;
    constantInt = _constant;
    defaultDefined = defineDefault;
    defaultIntDefined = defineDefault;
    defaultInt = _default;
    minimum = _min;
    maximum = _max;
}

void sdfData::setArrayData(float _minItems, float _maxItems,
        bool _uniqueItems, sdfData *_itemConstr)
{
    if (simpleType != json_array && simpleType != json_type_undef)
    {
        cerr << this->getLabel() + " cannot be instantiated as type array"
                + " because it already has a different type. "
                + "Please use setSimpType() first to reset it." << endl;
        return;
    }
    if (_itemConstr != NULL && _itemConstr->getType() == "array")
    {
        cerr << "setArrayData(): arrays cannot have items of type array"
                << endl;
        //return;
    }
    if (_itemConstr != NULL &&
            (_itemConstr->constDefined
            || _itemConstr->defaultDefined
            || _itemConstr->pattern != ""
            || !isnan(_itemConstr->multipleOf)
            || !isnan(_itemConstr->exclusiveMaximum_number)
            || !isnan(_itemConstr->exclusiveMinimum_number)
            || _itemConstr->exclusiveMaximum_bool
            || _itemConstr->exclusiveMinimum_bool))
        cerr << "setArrayData(): item constraints contain invalid attributes"
        << endl;
    this->setSimpType(json_array);
    minItems = _minItems;
    maxItems = _maxItems;
    uniqueItems = _uniqueItems;
    item_constr = _itemConstr;
}
/*
void sdfData::setArrayData(vector<string> enm, float _minItems,
        float _maxItems, bool _uniqueItems, string item_type, sdfCommon *ref,
        float minLength, float maxLength, jsonSchemaFormat format)
{
    this->setArrayData(_minItems, _maxItems, _uniqueItems, item_type, ref);

    item_constr = new sdfData("", "", item_type, ref);
    if (item_constr->getSimpType() == json_string)
    {
        item_constr->setStringData("", "", minLength, maxLength,
                "", format, enm);
    }
}

void sdfData::setArrayData(vector<float> enm, float _minItems,
        float _maxItems, bool _uniqueItems, string item_type, sdfCommon *ref,
        float min, float max)
{
    this->setArrayData(_minItems, _maxItems, _uniqueItems, item_type, ref);

    item_constr = new sdfData("", "", item_type, ref);
    if (item_constr->getSimpType() == json_number)
    {
        item_constr->setNumberData(NAN, NAN, min, max, NAN, enm);
    }
}

void sdfData::setArrayData(vector<int> enm, float _minItems,
        float _maxItems, bool _uniqueItems, string item_type, sdfCommon *ref,
        float min, float max)
{
    this->setArrayData(_minItems, _maxItems, _uniqueItems, item_type, ref);

    item_constr = new sdfData("", "", item_type, ref);
    if (item_constr->getSimpType() == json_integer)
    {
        item_constr->setIntData(-1, false, -1, false, min, max, enm);
    }
}
*/
void sdfData::setUnits(string _units, float _scaleMin, float _scaleMax)
{
    units = _units;
    scaleMinimum = _scaleMin;
    scaleMaximum = _scaleMax;
}

void sdfData::setReadWrite(bool _readable, bool _writable)
{
    readable = _readable;
    writable = _writable;
}

void sdfData::setObserveNull(bool _observable, bool _nullable)
{
    observable = _observable;
    nullable = _nullable;
}

void sdfData::setFormat(jsonSchemaFormat _format)
{
    format = _format;
}

void sdfData::setSubtype(sdfSubtype _subtype)
{
    subtype = _subtype;
}

bool sdfData::getConstantBool()
{
    if (!constDefined)
        cerr << "Constant value of sdfData " + this->getLabel()
        + " was queried but is undefined" << endl;
    return constantBool;
}

int sdfData::getConstantInt()
{
    if (!constDefined)
        cerr << "Constant value of sdfData " + this->getLabel()
        + " was queried but is undefined" << endl;
    return constantInt;
}

float sdfData::getConstantNumber()
{
    if (!constDefined)
        cerr << "Constant value of sdfData " + this->getLabel()
        + " was queried but is undefined" << endl;
    return constantNumber;
}

string sdfData::getConstantString()
{
    if (!constDefined)
        cerr << "Constant value of sdfData " + this->getLabel()
        + " was queried but is undefined" << endl;
    return constantString;
}

string sdfData::getContentFormat()
{
    return contentFormat;
}

bool sdfData::getDefaultBool()
{
    if (!defaultDefined)
        cerr << "Default value of sdfData " + this->getLabel()
        + " was queried but is undefined" << endl;
    return defaultBool;
}

int sdfData::getDefaultInt()
{
    if (!defaultDefined)
        cerr << "Default value of sdfData " + this->getLabel()
        + " was queried but is undefined" << endl;
    return defaultInt;
}

float sdfData::getDefaultNumber()
{
    if (!defaultDefined)
        cerr << "Default value of sdfData " + this->getLabel()
        + " was queried but is undefined" << endl;
    return defaultNumber;
}

string sdfData::getDefaultString()
{
    if (!defaultDefined)
        cerr << "Default value of sdfData " + this->getLabel()
        + " was queried but is undefined" << endl;
    return defaultString;
}
/*
vector<bool> sdfData::getEnumBool()
{
    return enumBool;
}

vector<int> sdfData::getEnumInt()
{
    return enumInt;
}

vector<float> sdfData::getEnumNumber()
{
    return enumNumber;
}
*/
vector<string> sdfData::getEnumString()
{
    return enumString;
}

bool sdfData::isExclusiveMaximumBool()
{
    return exclusiveMaximum_bool;
}

float sdfData::getExclusiveMaximumNumber()
{
    return exclusiveMaximum_number;
}

bool sdfData::isExclusiveMinimumBool()
{
    return exclusiveMinimum_bool;
}

float sdfData::getExclusiveMinimumNumber()
{
    return exclusiveMinimum_number;
}

jsonSchemaFormat sdfData::getFormat()
{
    return format;
}

float sdfData::getMaximum()
{
    return maximum;
}

float sdfData::getMaxItems()
{
    return maxItems;
}

float sdfData::getMaxLength()
{
    return maxLength;
}

float sdfData::getMinimum()
{
    return minimum;
}

float sdfData::getMinItems()
{
    return minItems;
}

float sdfData::getMinLength()
{
    return minLength;
}

float sdfData::getMultipleOf()
{
    return multipleOf;
}

bool sdfData::getNullable()
{
    return nullable;
}

bool sdfData::getObservable()
{
    return observable;
}

string sdfData::getPattern()
{
    return pattern;
}

float sdfData::getScaleMaximum()
{
    return scaleMaximum;
}

float sdfData::getScaleMinimum()
{
    return scaleMinimum;
}

sdfSubtype sdfData::getSubtype()
{
    return subtype;
}

string sdfData::getType()
{
    // TODO: do this?
    if (this->getReference())
    {
        if (simpleType != json_type_undef)
            cerr << "sdfData::getType: both reference and type given"
            << endl;
        sdfData *ref = dynamic_cast<sdfData*>(this->getReference());
        if (ref)
            return ref->getType();
        else
            cerr << "sdfData::getType: reference is of wrong type" << endl;
    }

    if (this->simpleType == json_type_undef)
        return jsonDTypeToString(simpleType);

    return this->derType;
}

jsonDataType sdfData::getSimpType()
{
    // TODO: do this?
    if (this->getReference())
    {
        if (simpleType != json_type_undef)
            cerr << "sdfData::getSimpType: both reference and type given"
            << endl;
        sdfData *ref = dynamic_cast<sdfData*>(this->getReference());
        if (ref)
            return ref->getSimpType();
        else
            cerr << "sdfData::getSimpType: reference is of wrong type" << endl;
    }

    return simpleType;
}

bool sdfData::getUniqueItems()
{
    return uniqueItems;
}

string sdfData::getUnits()
{
    return units;
}

string sdfData::generateReferenceString()
{
    sdfCommon *parent = this->getParentCommon();

    if (!parent)
    {
        cerr << "sdfData::generateReferenceString(): sdfData object "
                + this->getLabel()
                + " has no assigned parent. Possibly it was only"
                + " added as an output or input data reference and not"
                + " as a datatype definition."
                << endl;
        return "";
    }
    else
    {
        sdfAction *a = dynamic_cast<sdfAction*>(parent);
        sdfEvent *e = dynamic_cast<sdfEvent*>(parent);

        // if the parent is another sdfData object
        if (dynamic_cast<sdfData*>(parent) && !dynamic_cast<sdfObject*>(parent)
                && !a && !e)
            return parent->generateReferenceString()
                    + "/" + this->getLabel();

        // else if this is the input data of an sdfAction object
        else if (a && a->getInputData() == this)
            return parent->generateReferenceString()
                    + "/sdfInputData";

        // else if this is the output data of an sdfAction object
        // OR if this is the output data of an sdfEvent object
        else if ((a && a->getOutputData() == this)
                || (e && e->getOutputData() == this))
            return parent->generateReferenceString()
                    + "/sdfOutputData";

        else
            return parent->generateReferenceString()
                    + "/sdfData/" + this->getLabel();
    }
}

json sdfData::dataToJson(json prefix)
{
    json data;
    data = this->commonToJson(data);

    // TODO: really only print sdfRef when it is given?
    // No, because extensions of a referenced element are possible
    //if (this->getReference() == NULL)

    //cout << this->getLabel() << endl;
    if (this->getUnits() != "")
        data["unit"] = this->getUnits();
    if (this->getSubtype() == sdf_byte_string)
        data["sdfType"] = "byte-string";
    else if (this->getSubtype() == sdf_unix_time)
            data["sdfType"] = "unix-time";
    // TODO: what exactly is the content format??
    if (this->getContentFormat() != "")
        data["contentFormat"] = this->getContentFormat();
    // TODO: when should these be printed?
    //if (!this->getReadable())
    if (this->readableDefined)
        data["readable"] = this->getReadable();
    //if (!this->getWritable())
    if (this->writableDefined)
        data["writable"] = this->getWritable();
    //if (!this->getObservable())
    if (this->observableDefined)
        data["observable"] = this->getObservable();
    //if (!this->getNullable())
    if (this->nullableDefined)
        data["nullable"] = this->getNullable();
    //if (!this->sdfChoice.empty())
    {
        for (sdfData *i : sdfChoice)
        {
            json tmpJson;
            data["sdfChoice"][i->getLabel()]
                            = i->dataToJson(tmpJson)["sdfData"][i->getLabel()];
        }
    }

    // TODO: scaleMinimum and scaleMaximum (where do they go?)
    // TODO: exclusiveMinimum and exclusiveMaximum (bool and float)

    if (constDefined)
    {
        if (!isnan(this->getConstantNumber()))
            data["const"] = this->getConstantNumber();
        else if (this->getConstantString() != "")
            data["const"] = this->getConstantString();
        else if (constBoolDefined)
            data["const"] = this->getConstantBool();
        else if (constIntDefined)
            data["const"] = this->getConstantInt();
        else if (!this->getConstantNumberArray().empty())
            data["const"] = this->getConstantNumberArray();
        else if (!this->getConstantStringArray().empty())
            data["const"] = this->getConstantStringArray();
        else if (!this->getConstantBoolArray().empty())
            data["const"] = this->getConstantBoolArray();
        else if (!this->getConstantIntArray().empty())
            data["const"] = this->getConstantIntArray();
        else if (this->getConstantObject())
        {
            json tmpJson;
            data["const"]
                 = this->getConstantObject()->dataToJson(tmpJson)["sdfData"];
        }
    }
    if (defaultDefined)
    {
        if (!isnan(this->getDefaultNumber()))
            data["default"] = this->getDefaultNumber();
        else if (this->getDefaultString() != "")
            data["default"] = this->getDefaultString();
        else if (defaultBoolDefined)
            data["default"] = this->getDefaultBool();
        else if (defaultIntDefined)
            data["default"] = this->getDefaultInt();
        else if (!this->getDefaultNumberArray().empty())
            data["default"] = this->getDefaultNumberArray();
        else if (!this->getDefaultStringArray().empty())
            data["default"] = this->getDefaultStringArray();
        else if (!this->getDefaultBoolArray().empty())
            data["default"] = this->getDefaultBoolArray();
        else if (!this->getDefaultIntArray().empty())
            data["default"] = this->getDefaultIntArray();
        else if (this->getDefaultObject())
        {
            json tmpJson;
            data["default"]
                 = this->getDefaultObject()->dataToJson(tmpJson)["sdfData"];
        }
    }
    if (!isnan(this->getMinimum()))
        data["minimum"] = this->getMinimum();
    if (!isnan(this->getMaximum()))
        data["maximum"] = this->getMaximum();
    if (!isnan(this->getMultipleOf()))
        data["multipleOf"] = this->getMultipleOf();
    if (!this->getEnumString().empty())
        data["enum"] = this->getEnumString();
    if (!isnan(this->getMinLength()))
        data["minLength"] = this->getMinLength();
    if (!isnan(this->getMaxLength()))
        data["maxLength"] = this->getMaxLength();
    if (this->getPattern() != "")
        data["pattern"] = this->getPattern();
    if (!this->getEnumString().empty())
        data["enum"] = this->getEnumString();
    if (!this->getFormat() == json_format_undef)
        data["format"] = this->getFormat();
    if (!isnan(this->getMinItems()))
        data["minItems"] = this->getMinItems();
    if (!isnan(this->getMaxItems()))
        data["maxItems"] = this->getMaxItems();
    //if (this->getUniqueItems())
    if (this->uniqueItemsDefined)
        data["uniqueItems"] = this->getUniqueItems();
    if (this->item_constr != NULL)
    {
        json tmpJson;
        data["items"] = item_constr->dataToJson(tmpJson)
             ["sdfData"][item_constr->getLabel()];
    }
    for (sdfData *i : this->getObjectProperties())
    {
        json tmpJson;
        data["properties"][i->getLabel()]
                 = i->dataToJson(tmpJson)["sdfData"][i->getLabel()];
    }
    if (!requiredObjectProperties.empty())
        data["required"] = requiredObjectProperties;

    switch (simpleType)
    {
    case json_type_undef:
        //if (derType != "")
        //    data["type"] = derType;
        break;
    case json_number:
        if (this->derType  == "")
            data["type"] = "number";
        break;
    case json_string:
        if (this->derType  == "")
            data["type"] = "string";
        break;
    case json_boolean:
        if (this->derType  == "")
            data["type"] = "boolean";
        break;
    case json_integer:
        if (this->derType  == "")
            data["type"] = "integer";
        if (!isnan(this->getMinimum()))
            data["minimum"] = (int)this->getMinimum();
        if (!isnan(this->getMaximum()))
            data["maximum"] = (int)this->getMaximum();
        break;
    case json_array:
        if (this->derType  == "")
            data["type"] = "array";
        break;
    case json_object:
        if (this->derType == "")
            data["type"] = "object";
        break;
    }

    /*switch (simpleType)
    {
    case json_type_undef:
        if (derType != "")
            data["type"] = derType;
        break;
    case json_number:
        if (this->derType  == "")
            data["type"] = "number";
        if (constDefined)
            data["const"] = this->getConstantNumber();
        if (defaultDefined)
            data["default"] = this->getDefaultNumber();
        if (!isnan(this->getMinimum()))
            data["minimum"] = this->getMinimum();
        if (!isnan(this->getMaximum()))
            data["maximum"] = this->getMaximum();
        if (!isnan(this->getMultipleOf()))
            data["multipleOf"] = this->getMultipleOf();
        //if (!this->getEnumNumber().empty())
        //    data["enum"] = this->getEnumNumber();
        break;
    case json_string:
        if (this->derType  == "")
            data["type"] = "string";
        if (!this->getEnumString().empty())
            data["enum"] = this->getEnumString();
        if (constDefined)
            data["const"] = this->getConstantString();
        if (defaultDefined)
            data["default"] = this->getDefaultString();
        if (!isnan(this->getMinLength()))
            data["minLength"] = this->getMinLength();
        if (!isnan(this->getMaxLength()))
            data["maxLength"] = this->getMaxLength();
        if (this->getPattern() != "")
            data["pattern"] = this->getPattern();
        if (!this->getEnumString().empty())
            data["enum"] = this->getEnumString();
        if (!this->getFormat() == json_format_undef)
            data["format"] = this->getFormat();
        break;
    case json_boolean:
        if (this->derType  == "")
            data["type"] = "boolean";
        if (this->constDefined)
            data["const"] = this->getConstantBool();
        if (this->defaultDefined)
            data["default"] = this->getDefaultBool();
        //if (!this->getEnumBool().empty())
        //    data["enum"] = this->getEnumBool();
        break;
    case json_integer:
        if (this->derType  == "")
            data["type"] = "integer";
        if (this->constDefined)
            data["const"] = this->getConstantInt();
        if (this->defaultDefined)
            data["default"] = this->getDefaultInt();
        if (!isnan(this->getMinimum()))
            data["minimum"] = (int)this->getMinimum();
        if (!isnan(this->getMaximum()))
            data["maximum"] = (int)this->getMaximum();
        //if (!this->getEnumInt().empty())
        //    data["enum"] = this->getEnumInt();
        break;
    case json_array:
        if (this->derType  == "")
            data["type"] = "array";
        if (!isnan(this->getMinItems()))
            data["minItems"] = this->getMinItems();
        if (!isnan(this->getMaxItems()))
            data["maxItems"] = this->getMaxItems();
        if (this->getUniqueItems())
            data["uniqueItems"] = this->getUniqueItems();
        if (this->item_constr != NULL)
        {
            json tmpJson = data;
            data["items"]
                 = item_constr->dataToJson(tmpJson["items"])["sdfData"][""];
        }
        break;
    }*/

    prefix["sdfData"][this->getLabel()] = data;
    //if (this->getLabel() == "temperature")
        //cout << data.dump(4)<<endl;

    //7cout << "    " << this->getLabel() << endl;
    return prefix;
}

sdfEvent::sdfEvent(string _label, string _description, sdfCommon *_reference,
        vector<sdfCommon*> _required, sdfObject *_parentObject,
        sdfData* _outputData, vector<sdfData*> _datatypes)
            : sdfObjectElement(_label, _description, _reference, _required),
              sdfCommon(_label, _description, _reference, _required),
              outputData(_outputData), datatypes(_datatypes)
{
    this->setParentObject(_parentObject);
}
sdfEvent::~sdfEvent()
{
    delete outputData;
    delete[] datatypes.data();
}

void sdfEvent::setOutputData(sdfData *outputData)
{
    this->outputData = outputData;
    if (outputData)
        this->outputData->setParentCommon(this);
    else
        cerr << "sdfEvent::setOutputData: empty output data" << endl;
}

void sdfEvent::addDatatype(sdfData *datatype)
{
    this->datatypes.push_back(datatype);
    datatype->setParentCommon((sdfCommon*)this);
}

vector<sdfData*> sdfEvent::getDatatypes()
{
    return this->datatypes;
}

sdfData* sdfEvent::getOutputData()
{
    return this->outputData;
}

string sdfEvent::generateReferenceString()
{
    return this->sdfObjectElement::generateReferenceString()
        + "sdfEvent/" + this->getLabel();
}

json sdfEvent::eventToJson(json prefix)
{
    prefix["sdfEvent"][this->getLabel()]
                     = this->commonToJson(prefix["sdfEvent"][this->getLabel()]);
    for (sdfData *i : this->getDatatypes())
    {
        prefix["sdfEvent"][this->getLabel()]
                     = i->dataToJson(prefix["sdfEvent"][this->getLabel()]);
    }
    // TODO: change according to new SDF version (sdfOutputData is not a
    // pointer-list anymore)
    if (outputData != NULL)
    {
        json tmp;
        prefix["sdfEvent"][this->getLabel()]["sdfOutputData"]
          = outputData->dataToJson(tmp)["sdfData"][outputData->getLabel()];
        //prefix["sdfEvent"][this->getLabel()]["sdfOutputData"][i->getLabel()]["sdfRef"]
        //             = i->generateReferenceString();
    }
    return prefix;
}

void sdfAction::setInputData(sdfData *inputData)
{
    this->inputData = inputData;
    if (inputData)
        this->inputData->setParentCommon(this);
    else
        cerr << "sdfAction::setInputData: empty input data" << endl;
}

void sdfAction::addRequiredInputData(sdfData *requiredInputData)
{
    this->requiredInputData.push_back(requiredInputData);
}

void sdfAction::setOutputData(sdfData *outputData)
{
    this->outputData = outputData;
    if (outputData)
        this->outputData->setParentCommon(this);
    else
        cerr << "sdfAction::setOutputData: empty output data" << endl;
}

void sdfAction::addDatatype(sdfData *datatype)
{
    this->datatypes.push_back(datatype);
    datatype->setParentCommon((sdfCommon*)this);
}

sdfAction::sdfAction(string _label, string _description, sdfCommon *_reference,
        vector<sdfCommon*> _required, sdfObject *_parentObject,
        sdfData* _inputData, vector<sdfData*> _requiredInputData,
        sdfData* _outputData, vector<sdfData*> _datatypes)
            : sdfCommon(_label, _description, _reference, _required),
              inputData(_inputData), requiredInputData(_requiredInputData),
              outputData(_outputData), datatypes(_datatypes)
{
    this->setParentObject(_parentObject);
}

sdfAction::~sdfAction()
{
    delete inputData;
    delete[] requiredInputData.data();
    delete outputData;
    delete[] datatypes.data();
}

sdfData* sdfAction::getInputData()
{
    return this->inputData;
}

vector<sdfData*> sdfAction::getRequiredInputData()
{
    return this->requiredInputData;
}

sdfData* sdfAction::getOutputData()
{
    return this->outputData;
}

vector<sdfData*> sdfAction::getDatatypes()
{
    return this->datatypes;
}

string sdfAction::generateReferenceString()
{
    return this->sdfObjectElement::generateReferenceString()
        + "sdfAction/" + this->getLabel();
}

json sdfAction::actionToJson(json prefix)
{
    prefix["sdfAction"][this->getLabel()]
            = this->commonToJson(prefix["sdfAction"][this->getLabel()]);

    for (sdfData *i : this->getDatatypes())
    {
        prefix["sdfAction"][this->getLabel()]
                     = i->dataToJson(prefix["sdfAction"][this->getLabel()]);
    }/*
    for (sdfData *i : this->getInputData())
    {
        prefix["sdfAction"][this->getLabel()]["sdfInputData"][i->getLabel()]["sdfRef"]
                             = i->generateReferenceString();
    }*/
    if (inputData != NULL)
    {
        json tmp;
        prefix["sdfAction"][this->getLabel()]["sdfInputData"]
                 = inputData->dataToJson(tmp)["sdfData"][inputData->getLabel()];
    }
    // TODO: obsolete?
    /*
    for (sdfData *i : this->getRequiredInputData())
    {
        prefix["sdfAction"][this->getLabel()]["sdfRequiredInputData"][i->getLabel()]["sdfRef"]
                             = i->generateReferenceString();
    }
    for (sdfData *i : this->getOutputData())
    {
        prefix["sdfAction"][this->getLabel()]["sdfOutputData"][i->getLabel()]["sdfRef"]
                     = i->generateReferenceString();
    }*/
    if (outputData != NULL)
    {
        json tmp;
        prefix["sdfAction"][this->getLabel()]["sdfOutputData"]
               = outputData->dataToJson(tmp)["sdfData"][outputData->getLabel()];
    }

    return prefix;
}

sdfProperty::sdfProperty(string _label, string _description,
        jsonDataType _type, sdfCommon *_reference,
        vector<sdfCommon*> _required, sdfObject *_parentObject)
        : sdfData(_label, _description, _type, _reference, _required),
          sdfObjectElement(_label, _description, _reference, _required),
          sdfCommon(_label, _description, _reference, _required)
{
    this->setParentObject(_parentObject);
    this->setParentCommon(NULL);
}

sdfProperty::sdfProperty(sdfData &data)
: sdfData(data),
  sdfObjectElement(data.getLabel(), data.getDescription(), data.getReference(), data.getRequired()),
  sdfCommon(data.getLabel(), data.getDescription(), data.getReference(), data.getRequired())
{
    this->setParentObject(NULL);
    this->setParentCommon(NULL);
    for (sdfData *props : this->getObjectProperties())
        props->setParentCommon(this);
    for (sdfData *choice : this->getChoice())
        choice->setParentCommon(this);
}

string sdfProperty::generateReferenceString()
{
    return this->sdfObjectElement::generateReferenceString()
        + "sdfProperty/" + this->getLabel();
}

json sdfProperty::propertyToJson(json prefix)
{
    json output = prefix;
    output =  this->dataToJson(output);
    prefix["sdfProperty"][this->getLabel()]
                          = output["sdfData"][this->getLabel()];
    return prefix;
}

sdfObject::sdfObject(string _label, string _description, sdfCommon *_reference,
        vector<sdfCommon*> _required, vector<sdfProperty*> _properties,
        vector<sdfAction*> _actions, vector<sdfEvent*> _events,
        vector<sdfData*> _datatypes, sdfThing *_parentThing)
            : sdfCommon(_label, _description, _reference, _required),
              properties(_properties), actions(_actions), events(_events),
              datatypes(_datatypes)
{
    this->setParentThing(_parentThing);
}

sdfObject::~sdfObject()
{
    delete[] properties.data();
    delete[] actions.data();
    delete[] events.data();
    delete[] datatypes.data();

    delete ns;
    delete info;
    delete parent;
}

void sdfObject::setInfo(sdfInfoBlock *_info)
{
    this->info = _info;
}

void sdfObject::setNamespace(sdfNamespaceSection *_ns)
{
    this->ns = _ns;
}

void sdfObject::addProperty(sdfProperty *property)
{
    this->properties.push_back(property);
    property->setParentObject(this);
}

void sdfObject::addAction(sdfAction *action)
{
    this->actions.push_back(action);
    action->setParentObject(this);
}

void sdfObject::addEvent(sdfEvent *event)
{
    this->events.push_back(event);
    event->setParentObject(this);
}

void sdfObject::addDatatype(sdfData *datatype)
{
    this->datatypes.push_back(datatype);
    datatype->setParentCommon((sdfCommon*)this);
    //datatype->setParentCommon(this);

}

sdfInfoBlock* sdfObject::getInfo()
{
    return this->info;
}

sdfNamespaceSection* sdfObject::getNamespace()
{
    return this->ns;
}

vector<sdfProperty*> sdfObject::getProperties()
{
    return this->properties;
}

vector<sdfAction*> sdfObject::getActions()
{
    return this->actions;
}

vector<sdfEvent*> sdfObject::getEvents()
{
    return this->events;
}

vector<sdfData*> sdfObject::getDatatypes()
{
    return this->datatypes;
}

sdfThing* sdfObject::getParentThing()
{
    return parent;
}

void sdfObject::setParentThing(sdfThing *parentThing)
{
    this->parent = parentThing;
    // TODO: also add this to parentThings object list?
    // make sure the object is in the list only once
    // parentThing->addObject(this);
}

json sdfObject::objectToJson(json prefix, bool print_info_namespace)
{
    // print info if specified by print_info
    if (print_info_namespace)
    {
        if (this->info != NULL)
            prefix = this->info->infoToJson(prefix);
        if (this->ns != NULL)
            prefix = this->ns->namespaceToJson(prefix);
    }

    prefix["sdfObject"][this->getLabel()]
                    = this->commonToJson(prefix["sdfObject"][this->getLabel()]);

    for (sdfData *i : this->getDatatypes())
    {
        prefix["sdfObject"][this->getLabel()]
                    = i->dataToJson(prefix["sdfObject"][this->getLabel()]);
    }
    for (sdfProperty *i : this->getProperties())
    {
        prefix["sdfObject"][this->getLabel()]
                    = i->propertyToJson(prefix["sdfObject"][this->getLabel()]);
    }
    for (sdfAction *i : this->getActions())
    {
        prefix["sdfObject"][this->getLabel()]
                    = i->actionToJson(prefix["sdfObject"][this->getLabel()]);
    }
    for (sdfEvent *i : this->getEvents())
    {
        prefix["sdfObject"][this->getLabel()]
                    = i->eventToJson(prefix["sdfObject"][this->getLabel()]);
    }

    return prefix;
}

string sdfObject::objectToString(bool print_info_namespace)
{
    json json_output;
    return this->objectToJson(
            json_output, print_info_namespace).dump(INDENT_WIDTH);
}

void sdfObject::objectToFile(string path)
{
    std::ofstream output(path);
    if (output)
    {
        output << this->objectToString(true) << std::endl;
        output.close();
    }
    else
        cerr << "Error opening file" << endl;
}

string sdfObject::generateReferenceString()
{
    if (this->parent != NULL)
        return this->parent->generateReferenceString() + "/sdfObject/"
                + this->getLabel();
    else
        return "#/sdfObject/" + this->getLabel();
}

sdfThing::sdfThing(string _label, string _description, sdfCommon *_reference,
        vector<sdfCommon*> _required, vector<sdfThing*> _things,
        vector<sdfObject*> _objects, sdfThing *_parentThing)
            : sdfCommon(_label, _description, _reference, _required),
              childThings(_things)//, childObjects(_objects)
{
    this->setParentThing(_parentThing);
}

sdfThing::~sdfThing()
{
    delete[] childThings.data();
    delete[] childObjects.data();
    delete ns;
    delete info;
    delete parent;
}

void sdfThing::setInfo(sdfInfoBlock *_info)
{
    this->info = _info;
}

void sdfThing::setNamespace(sdfNamespaceSection *_ns)
{
    this->ns = _ns;
}

sdfInfoBlock* sdfThing::getInfo() const
{
    return this->info;
}

sdfNamespaceSection* sdfThing::getNamespace() const
{
    return this->ns;
}

void sdfThing::addThing(sdfThing *thing)
{
    this->childThings.push_back(thing);
    thing->setParentThing(this);
}

void sdfThing::addObject(sdfObject *object)
{
    this->childObjects.push_back(object);
    object->setParentThing(this);
}

vector<sdfThing*> sdfThing::getThings() const
{
    return this->childThings;
}

vector<sdfObject*> sdfThing::getObjects() const
{
    return this->childObjects;
}

json sdfThing::thingToJson(json prefix, bool print_info_namespace)
{
    // print info if specified by print_info
    if (print_info_namespace)
    {
        if (this->info != NULL)
            prefix = this->info->infoToJson(prefix);
        if (this->ns != NULL)
            prefix = this->ns->namespaceToJson(prefix);
    }
    if (this->getLabel() != "")
        prefix["sdfThing"][this->getLabel()]
                = this->commonToJson(prefix["sdfThing"][this->getLabel()]);
    for (sdfThing *i : this->childThings)
    {
        prefix["sdfThing"][this->getLabel()] = i->thingToJson(prefix["sdfThing"][this->getLabel()], false);
    }

    for (sdfObject *i : this->childObjects)
    {
        prefix["sdfThing"][this->getLabel()] = i->objectToJson(prefix["sdfThing"][this->getLabel()], false);
    }
    return prefix;
}

string sdfThing::thingToString(bool print_info_namespace)
{
    json json_output;
    return this->thingToJson(
            json_output, print_info_namespace).dump(INDENT_WIDTH);
}

void sdfThing::thingToFile(string path)
{
    ofstream output(path);
    if (output)
    {
        output << this->thingToString() << endl;
        output.close();
    }
    else
        cerr << "Error opening file" << endl;
}

sdfThing* sdfThing::getParentThing() const
{
    return parent;
}

void sdfThing::setParentThing(sdfThing *parentThing)
{
    this->parent = parentThing;
}

string sdfThing::generateReferenceString()
{
    if (this->parent != NULL)
        return this->parent->generateReferenceString() + "/sdfThing/"
                + this->getLabel();
    else
        return "#/sdfThing/" + this->getLabel();
}

void sdfCommon::jsonToCommon(json input)
{
    for (auto it : input.items())
    {
        //cout << "jsonToCommon: " << it.key() << endl;
        if (it.key() == "label")
            this->setLabel(correctValue(it.value()));
        else if (it.key() == "description")
            this->setDescription(it.value());
        else if (it.key() == "sdfRef")
            this->reference = refToCommon(correctValue(it.value()));
        else if (it.key() == "sdfRequired")
            for (auto jt : it.value())
                if (refToCommon(jt) != NULL)
                    this->addRequired(refToCommon(jt));
    }
    existingDefinitons[this->generateReferenceString()] = this;
    //cout << "jsonToCommon: " << this->generateReferenceString() << endl;
}

sdfData* sdfData::jsonToData(json input)
{
    this->jsonToCommon(input);
    for (json::iterator it = input.begin(); it != input.end(); ++it)
    {
        if (it.key() == "type" && !it.value().empty())
        {
            this->setType((string)input["type"]);
        }
        else if (it.key() == "enum" && !it.value().empty())
        {
            if (it.value().is_array())
            {
                for (auto i : it.value())
                {
                    if (i.is_string())
                        this->enumString.push_back(i);
                    /*if (i.is_number_integer())
                        this->enumInt.push_back(i);
                    if (i.is_number())
                        this->enumNumber.push_back(i);
                    else if (i.is_boolean())
                        this->enumBool.push_back(i);
                    else if (i.is_array())
                        ; // TODO: fix this?*/
                }
            }
        }
        else if (it.key() == "sdfChoice" && !it.value().empty())
        {
            for (json::iterator jt = it.value().begin(); jt != it.value().end();
                    ++jt)
            {
                sdfData *choice = new sdfData();
                choice->setLabel(correctValue(jt.key()));
                this->addChoice(choice->jsonToData(jt.value()));
            }
        }
        else if (it.key() == "required" && !it.value().empty())
        {
            if (it.value().is_array() && it.value()[0].is_string())
            {
                for (auto i : it.value())
                    this->addRequiredObjectProperty((string)i);
            }
            else
                cerr << "jsonToData(): field \'required\' could not be parsed"
                << endl;
        }
        else if (it.key() == "properties" && !it.value().empty())
        {
            for (json::iterator jt = it.value().begin(); jt != it.value().end();
                                ++jt)
            {
                //cout << jt.key() << endl;
                sdfData *objectProperty = new sdfData();
                objectProperty->setLabel(correctValue(jt.key()));
                this->addObjectProperty(objectProperty->jsonToData(jt.value()));
            }
        }
        if (it.key() == "const" && !it.value().empty())
        {
            this->constDefined = true;
            if (it.value().is_number_integer())
            {
                this->constantInt = it.value();
                this->constIntDefined = true;
            }
            else if (it.value().is_number())
                this->constantNumber = it.value();
            else if (it.value().is_string())
                this->constantString = it.value();
            else if (it.value().is_boolean())
            {
                this->constantBool = it.value();
                this->constBoolDefined = true;
            }
            else if (it.value().is_array())
                ;// TODO: fix this
        }
        else if (it.key() == "default" && !it.value().empty())
        {
            this->defaultDefined = true;
            if (it.value().is_number_integer())
            {
                this->defaultInt = it.value();
                this->defaultIntDefined = true;
            }
            else if (it.value().is_number())
                this->defaultNumber = it.value();
            else if (it.value().is_string())
                this->defaultString = it.value();
            else if (it.value().is_boolean())
            {
                this->defaultBool = it.value();
                this->defaultBoolDefined = true;
            }
            else if (it.value().is_array())
                ;// TODO: fix this
        }
        else if (it.key() == "minimum" && !it.value().empty())
        {
            this->minimum = it.value();
        }
        else if (it.key() == "maximum" && !it.value().empty())
        {
            this->maximum = it.value();
        }
        else if (it.key() == "exclusiveMinimum" && !it.value().empty())
        {
            if (it.value().is_boolean())
                this->exclusiveMinimum_bool = it.value();
            else if (it.value().is_number())
                this->exclusiveMinimum_number = it.value();
        }
        else if (it.key() == "exclusiveMaximum" && !it.value().empty())
        {
            if (it.value().is_boolean())
                this->exclusiveMaximum_bool = it.value();
            else if (it.value().is_number())
                this->exclusiveMaximum_number = it.value();
        }
        else if (it.key() == "multipleOf" && !it.value().empty())
        {
            this->multipleOf = it.value();
        }
        else if (it.key() == "minLength" && !it.value().empty())
        {
            this->minLength = it.value();
        }
        else if (it.key() == "maxLength" && !it.value().empty())
        {
            this->maxLength = it.value();
        }
        else if (it.key() == "pattern" && !it.value().empty())
        {
            this->pattern = it.value();
        }
        else if (it.key() == "format" && !it.value().empty())
        {
            if (it.value() == "date-time")
                this->contentFormat = json_date_time;
            else if (it.value() == "date")
                this->contentFormat = json_date;
            else if (it.value() == "time")
                this->contentFormat = json_time;
            else if (it.value() == "uri")
                this->contentFormat = json_uri;
            else if (it.value() == "uri-reference")
                this->contentFormat = json_uri_reference;
            else if (it.value() == "uuid")
                this->contentFormat = json_uuid;
            else
                this->contentFormat = json_format_undef;
        }
        else if (it.key() == "minItems" && !it.value().empty())
        {
            this->minItems = it.value();
        }
        else if (it.key() == "maxItems" && !it.value().empty())
        {
            this->maxItems = it.value();
        }
        else if (it.key() == "uniqueItems" && !it.value().empty())
        {
            this->uniqueItems = it.value();
            this->uniqueItemsDefined = true;
        }
        else if (it.key() == "items" && !it.value().empty())
        {
            this->item_constr = new sdfData();
            this->item_constr->jsonToData(input["items"]);
        }
        // TODO: keep key "units" for older versions?
        else if ((it.key() == "unit" || it.key() == "units")
                && !it.value().empty())
        {
            this->units = it.value();
        }
        else if (it.key() == "scaleMinimum" && !it.value().empty())
        {
            this->scaleMinimum = it.value();
        }
        else if (it.key() == "scaleMaximum" && !it.value().empty())
        {
            this->scaleMaximum = it.value();
        }
        else if (it.key() == "readable" && !it.value().empty())
        {
            this->readable = it.value();
            this->readableDefined = true;
        }
        else if (it.key() == "writable" && !it.value().empty())
        {
            this->writable = it.value();
            this->writableDefined = true;
        }
        else if (it.key() == "observable" && !it.value().empty())
        {
            this->observable = it.value();
            this->observableDefined = true;
        }
        else if (it.key() == "nullable" && !it.value().empty())
        {
            this->nullable = it.value();
            this->nullableDefined = true;
        }
        else if (it.key() == "contentFormat" && !it.value().empty())
        {
            // TODO: ??
        }
        //else if (it.key() == "subtype" && !it.value().empty())
        // TODO: keep "subtype" for older versions?
        else if ((it.key() == "sdfType" || it.key() == "subtype")
                && !it.value().empty())
        {
            if (it.value() == "byte-string")
                this->subtype = sdf_byte_string;
            else if (it.value() == "unix-time")
                this->subtype = sdf_unix_time;
            else
                this->subtype = sdf_subtype_undef;
        }
    }
    this->jsonToCommon(input);
    return this;
}

sdfEvent* sdfEvent::jsonToEvent(json input)
{
    this->jsonToCommon(input);
    for (auto it : input.items())
    {
        if (it.key() == "sdfOutputData" && !it.value().empty())
        {
            /*
            for (json::iterator jt = it.value().begin(); jt != it.value().end(); ++jt)
            {
                cout << "HELLLO" << jt.key() << endl;
                sdfData *data = new sdfData();
                data->setLabel(correctValue(jt.key()));
                data->jsonToData(input["sdfOutputData"]);
                this->addOutputData(data);
            }*/
            sdfData *data = new sdfData();
            //data->setLabel("");
            data->jsonToData(input["sdfOutputData"]);
            this->setOutputData(data);

        }
        else if (it.key() == "sdfData" && !it.value().empty())
        {
            for (json::iterator jt = it.value().begin(); jt != it.value().end(); ++jt)
            {
                sdfData *childData = new sdfData();
                this->addDatatype(childData);
                childData->jsonToData(input["sdfData"][jt.key()]);
            }
        }
    }
    this->jsonToCommon(input);
    return this;
}

sdfAction* sdfAction::jsonToAction(json input)
{
    this->jsonToCommon(input);
    for (json::iterator it = input.begin(); it != input.end(); ++it)
    {
        if (it.key() == "sdfInputData" && !it.value().empty())
        {/*
            for (json::iterator jt = it.value().begin(); jt != it.value().end(); ++jt)
            {
                sdfData *refData = new sdfData();
                this->addInputData(refData);
            }*/
            sdfData *data = new sdfData();
            data->jsonToData(input["sdfInputData"]);
            this->setInputData(data);
        }
        else if (it.key() == "sdfRequiredInputData" && !it.value().empty())
        {
            for (json::iterator jt = it.value().begin(); jt != it.value().end(); ++jt)
            {
                sdfData *refData = new sdfData();
                this->addRequiredInputData(refData);
            }
        }
        else if (it.key() == "sdfOutputData" && !it.value().empty())
        {/*
            for (json::iterator jt = it.value().begin(); jt != it.value().end(); ++jt)
            {
                sdfData *refData = new sdfData();
                this->addOutputData(refData);
            }*/
            sdfData *data = new sdfData();
            data->jsonToData(input["sdfOutputData"]);
            this->setOutputData(data);
        }
        else if (it.key() == "sdfData" && !it.value().empty())
        {
            for (json::iterator jt = it.value().begin(); jt != it.value().end(); ++jt)
            {
                sdfData *childData = new sdfData();
                this->addDatatype(childData);
                childData->jsonToData(input["sdfData"][jt.key()]);
            }
        }
    }
    this->jsonToCommon(input);
    return this;
}

sdfProperty* sdfProperty::jsonToProperty(json input)
{
    this->jsonToCommon(input);
    this->jsonToData(input);
    this->jsonToCommon(input);
    return this;
}

sdfObject* sdfObject::jsonToObject(json input)
{
    this->jsonToCommon(input);
    for (json::iterator it = input.begin(); it != input.end(); ++it)
    {
        //cout << "jsonToObject: " << it.key() << endl;
        if (it.key() == "info" && !it.value().empty())
        {
            this->setInfo(new sdfInfoBlock());
            this->info->jsonToInfo(input["info"]);
        }
        else if (it.key() == "namespace" && !it.value().empty())
        {
            this->setNamespace(new sdfNamespaceSection());
            this->ns->jsonToNamespace(input);
        }
        // for first level
        else if (it.key() == "sdfObject" && !it.value().empty())
        {
            for (json::iterator jt = it.value().begin(); jt != it.value().end(); ++jt)
                this->jsonToObject(input["sdfObject"][jt.key()]);
        }
        else if (it.key() == "sdfData" && !it.value().empty())
        {
            for (json::iterator jt = it.value().begin(); jt != it.value().end(); ++jt)
            {
                sdfData *childData = new sdfData();
                this->addDatatype(childData);
                childData->setLabel(correctValue(jt.key()));
                childData->jsonToData(input["sdfData"][jt.key()]);
            }
        }
        else if (it.key() == "sdfProperty" && !it.value().empty())
        {
            for (json::iterator jt = it.value().begin(); jt != it.value().end(); ++jt)
            {
                sdfProperty *childProperty = new sdfProperty();
                this->addProperty(childProperty);
                childProperty->setLabel(correctValue(jt.key()));
                childProperty->jsonToProperty(input["sdfProperty"][jt.key()]);

                //childProperty->setLabel("hi_oh");//jt.key());
            }
        }
        else if (it.key() == "sdfAction" && !it.value().empty())
        {
            for (json::iterator jt = it.value().begin(); jt != it.value().end(); ++jt)
            {
                sdfAction *childAction = new sdfAction();
                this->addAction(childAction);
                childAction->setLabel(correctValue(jt.key()));
                childAction->jsonToAction(input["sdfAction"][jt.key()]);
            }
        }
        else if (it.key() == "sdfEvent" && !it.value().empty())
        {
            for (json::iterator jt = it.value().begin(); jt != it.value().end(); ++jt)
            {
                sdfEvent *childEvent =  new sdfEvent();
                this->addEvent(childEvent);
                // TODO: with label set this way, a label will be printed
                // even though there was just a "title" in the original
                childEvent->setLabel(correctValue(jt.key()));
                childEvent->jsonToEvent(input["sdfEvent"][jt.key()]);
            }
        }
        else if (it.key() == "sdfThing")
        {
            cerr << "jsonToObject(): incorrect sdfObject (sdfThing found)";
            return NULL;
        }
        /*
        else if (it.key() != "label" && it.key() != "description" &&
                it.key() != "sdfRef" && it.key() != "sdfRequired")
        {
            printf("deeper\n");
            this->jsonToObject(input[it.key()]);
        }*/
    }
    // jsonToCommon needs to be called twice because of sdfRequired
    // (which cannot be filled before the rest of the object)
    this->jsonToCommon(input);
    return this;
}

sdfObject* sdfObject::fileToObject(string path)
{
    json json_input;
    ifstream input(path);
    if (input)
    {
        input >>  json_input;
        input.close();
    }
    else
        cerr << "Error opening file" << endl;
    return this->jsonToObject(json_input);
}

/*
 * This has to be an extra function because it is otherwise not possible
 * to distinguish whether this is a first-level Thing or a nested thing
 */
sdfThing* sdfThing::jsonToNestedThing(json input)
{
    this->jsonToCommon(input);
    for (json::iterator it = input.begin(); it != input.end(); ++it)
    {
        if (it.key() == "sdfThing" && !it.value().empty())
        {
            for (json::iterator jt = it.value().begin();
                    jt != it.value().end(); ++jt)
            {
                sdfThing *childThing = new sdfThing();
                this->addThing(childThing);
                childThing->jsonToNestedThing(input["sdfThing"][jt.key()]);
            }
        }
        else if (it.key() == "sdfObject" && !it.value().empty())
        {
            for (json::iterator jt = it.value().begin();
                    jt != it.value().end(); ++jt)
            {
                sdfObject *childObject = new sdfObject();
                this->addObject(childObject);
                childObject->jsonToObject(input["sdfObject"][jt.key()]);
            }
        }
    }
    this->jsonToCommon(input);
    return this;
}

sdfThing* sdfThing::jsonToThing(json input)
{
    this->jsonToCommon(input);
    sdfObject *childObject;

    for (json::iterator it = input.begin(); it != input.end(); ++it)
    {
        //cout << "jsonToThing: " << it.key() << endl;
        if (it.key() == "info" && !it.value().empty())
        {
            this->setInfo(new sdfInfoBlock());
            this->info->jsonToInfo(input["info"]);
        }
        else if (it.key() == "namespace" && !it.value().empty())
        {
            this->setNamespace(new sdfNamespaceSection());
            this->ns->jsonToNamespace(input);
        }
        else if (it.key() == "defaultNamespace" && !it.value().empty())
        {
            this->ns->jsonToNamespace(input["namespace"]);
        }
        else if (it.key() == "sdfThing" && !it.value().empty())
        {
            for (json::iterator jt = it.value().begin();
                    jt != it.value().end(); ++jt)
            {
                //sdfThing *childThing;
                //childThing->jsonToThing(input["sdfThing"][jt.key()]);
                //this->addThing(childThing);
                this->jsonToNestedThing(input["sdfThing"][jt.key()]);
            }
        }
        else if (it.key() == "sdfObject" && !it.value().empty())
        {
            for (json::iterator jt = it.value().begin();
                    jt != it.value().end(); ++jt)
            {
                childObject = new sdfObject();
                this->addObject(childObject);
                childObject->jsonToObject(input["sdfObject"][jt.key()]);
            }
        }
    }
    this->jsonToCommon(input);
    return this;
}

sdfThing* sdfThing::fileToThing(string path)
{
    json json_input;
    ifstream input(path);
    if (input)
    {
        input >>  json_input;
        input.close();
    }
    else
        cerr << "Error opening file" << endl;
    return this->jsonToThing(json_input);
}
/*
void sdfCommon::setParentCommon(sdfCommon *parentCommon)
{
    //cout << parentCommon->getLabel() << endl;
    this->parent = parentCommon;
}

sdfCommon* sdfCommon::getParentCommon()
{
    return this->parent;
}
*/
void sdfCommon::setDescription(string dsc)
{
    this->description = dsc;
}

void sdfData::setSimpType(jsonDataType _type)
{
    this->simpleType = _type;
}

void sdfData::setDerType(string _type)
{
    //this->simpleType = stringToJsonDType(_type);
    this->derType = _type;
}

void sdfData::setType(jsonDataType _type)
{
    this->simpleType = _type;
}

void sdfData::setType(string _type)
{
    if (stringToJsonDType(_type) == json_type_undef)
        this->derType = _type;
    //TODO: else?
    this->simpleType = stringToJsonDType(_type);
}

bool sdfData::getReadable()
{
    return this->readable;
}

bool sdfData::getWritable()
{
    return this->writable;
}

void sdfData::setMaxItems(float maxItems)
{
    this->maxItems = maxItems;
}

void sdfData::setMinItems(float minItems)
{
    this->minItems = minItems;
}

void sdfData::setPattern(string pattern)
{
    this->pattern = pattern;
}

void sdfData::setConstantBool(bool constantBool)
{
    this->constantBool = constantBool;
    this->constDefined = true;
    this->constBoolDefined = true;
}

void sdfData::setConstantInt(int constantInt)
{
    this->constantInt = constantInt;
    this->constDefined = true;
    this->constIntDefined = true;
}

void sdfData::setConstantNumber(float constantNumber)
{
    this->constantNumber = constantNumber;
    this->constDefined = true;
}

void sdfData::setConstantString(string constantString)
{
    this->constantString = constantString;
    this->constDefined = true;
}

void sdfData::setDefaultBool(bool defaultBool)
{
    this->defaultBool = defaultBool;
    this->defaultDefined = true;
    this->defaultBoolDefined = true;
}

void sdfData::setDefaultInt(int defaultInt)
{
    this->defaultInt = defaultInt;
    this->defaultDefined = true;
    this->defaultIntDefined = true;
}

void sdfData::setDefaultNumber(float defaultNumber)
{
    this->defaultNumber = defaultNumber;
    this->defaultDefined = true;
}

void sdfData::setDefaultString(string defaultString)
{
    this->defaultString = defaultString;
    this->defaultDefined = true;
}

vector<string> sdfData::getConstantStringArray() const
{
    return constantStringArray;
}

void sdfData::setConstantArray(vector<string> constantArray)
{
    this->constantStringArray = constantArray;
    this->constDefined = true;
}

vector<string> sdfData::getDefaultStringArray() const
{
    return defaultStringArray;
}

void sdfData::setDefaultArray(vector<string> defaultArray)
{
    this->defaultStringArray = defaultArray;
    this->defaultDefined = true;
}

void sdfData::setParentCommon(sdfCommon *parentCommon)
{
    this->parent = parentCommon;
}

sdfCommon* sdfData::getParentCommon()
{
    return this->parent;
}

sdfInfoBlock* sdfInfoBlock::jsonToInfo(json input)
{
    for (json::iterator it = input.begin(); it != input.end(); ++it)
    {
        if (it.key() == "title" && !it.value().empty())
        {
            this->title = it.value();
        }
        else if (it.key() == "version" && !it.value().empty())
        {
            this->version = it.value();
        }
        else if (it.key() == "copyright" && !it.value().empty())
        {
            this->copyright = it.value();
        }
        else if (it.key() == "license" && !it.value().empty())
        {
            this->license = it.value();
        }
    }
    return this;
}

sdfNamespaceSection* sdfNamespaceSection::jsonToNamespace(json input)
{
    for (json::iterator it = input.begin(); it != input.end(); ++it)
    {
        if (it.key() == "namespace" && !it.value().empty())
        {
            for (json::iterator jt = it.value().begin(); jt != it.value().end(); ++jt)
            {
                if (!jt.value().empty())
                    this->namespaces[jt.key()] = jt.value();
            }
        }
        else if (it.key() == "defaultNamespace" && !it.value().empty())
            this->default_ns = it.value();
    }
    return this;
}

const char* sdfCommon::getDescriptionAsArray()
{
    if (description != "")
        return description.c_str();
    return NULL;
}

std::map<const char*, const char*> sdfNamespaceSection::getNamespacesAsArrays()
{
    std::map<const char*, const char*> output;
    std::map<std::string, std::string>::iterator it = namespaces.begin();
    while (it != namespaces.end())
    {
        output[it->first.c_str()] = it->second.c_str();
        it++;
    }
    return output;
}

const char* sdfNamespaceSection::getDefaultNamespaceAsArray()
{
    return this->default_ns.c_str();
}

const char* sdfInfoBlock::getTitleAsArray() const
{
    return this->title.c_str();
}

const char* sdfInfoBlock::getVersionAsArray() const
{
    return this->version.c_str();
}

const char* sdfInfoBlock::getCopyrightAsArray() const
{
    return this->copyright.c_str();
}

const char* sdfInfoBlock::getLicenseAsArray() const
{
    return this->license.c_str();
}

const char* sdfData::getUnitsAsArray() const
{
    return this->units.c_str();
}

const char* sdfData::getDefaultAsCharArray()
{
    if (!defaultDefined)
        return NULL;
    std::string *str;
    if (simpleType == json_number)
    {
        /*
        char buf[64];
        if (snprintf(buf, sizeof(buf), "%f", defaultNumber) > 0)
            defaultAsCharArray = buf;*/
        str = new std::string(std::to_string(defaultNumber));
        defaultAsCharArray = str->c_str();
    }
    else if (simpleType == json_string && defaultString != "")
        defaultAsCharArray = defaultString.c_str();

    else if (simpleType == json_boolean)
    {
        if (defaultBool == true)
            defaultAsCharArray = "true";
        else
            defaultAsCharArray = "false";
    }
    else if (simpleType == json_integer)
    {
        str = new std::string(std::to_string(defaultInt));
        defaultAsCharArray = str->c_str();
    }

    return defaultAsCharArray;
}

const char* sdfData::getConstantAsCharArray()
{
    if (!constDefined)
        return NULL;
    std::string str;
    if (simpleType == json_number)
    {
        str = std::to_string(constantNumber);
        constantAsCharArray = str.c_str();
    }
    else if (simpleType == json_string && constantString != "")
        constantAsCharArray = constantString.c_str();

    else if (simpleType == json_boolean)
    {
        if (constantBool == true)
            constantAsCharArray = "true";
        else
            constantAsCharArray = "false";
    }
    else if (simpleType == json_integer)
    {
        str = std::to_string(constantInt);
        constantAsCharArray = str.c_str();
    }

    return constantAsCharArray;
}

sdfData* sdfData::getItemConstr() const
{
    return item_constr;
}

void sdfData::addChoice(sdfData *choice)
{
    choice->setParentCommon(this);
    sdfChoice.push_back(choice);
}

void sdfData::addObjectProperty(sdfData *property)
{
    objectProperties.push_back(property);
    property->setParentCommon((sdfCommon*)this);
}

void sdfData::addRequiredObjectProperty(string propertyName)
{
    requiredObjectProperties.push_back(propertyName);
}

std::vector<sdfData*> sdfData::getChoice() const
{
    return sdfChoice;
}

void sdfData::setChoice(vector<sdfData*> choices)
{
    sdfChoice = choices;
    for (sdfData *choice : this->getChoice())
        choice->setParentCommon(this);
}

void sdfData::setConstantArray(std::vector<bool> constantArray)
{
    constantBoolArray = constantArray;
    constDefined = true;
}

void sdfData::setDefaultArray(std::vector<bool> defaultArray)
{
    defaultBoolArray = defaultArray;
    defaultDefined = true;
}

void sdfData::setConstantArray(std::vector<int> constantArray)
{
    constantIntArray = constantArray;
    constDefined = true;
}

void sdfData::setDefaultArray(std::vector<int> defaultArray)
{
    defaultIntArray = defaultArray;
    defaultDefined = true;
}

void sdfData::setConstantArray(std::vector<float> constantArray)
{
    constantNumberArray = constantArray;
    constDefined = true;
}

void sdfData::setDefaultArray(std::vector<float> defaultArray)
{
    defaultNumberArray = defaultArray;
    defaultDefined = true;
}

std::vector<bool> sdfData::getConstantBoolArray() const
{
    return constantBoolArray;
}

std::vector<bool> sdfData::getDefaultBoolArray() const
{
    return defaultBoolArray;
}

std::vector<int> sdfData::getConstantIntArray() const
{
    return constantIntArray;
}

std::vector<int> sdfData::getDefaultIntArray() const
{
    return defaultIntArray;
}

std::vector<float> sdfData::getDefaultNumberArray() const
{
    return defaultNumberArray;
}

std::vector<float> sdfData::getConstantNumberArray() const
{
    return constantNumberArray;
}

std::vector<sdfData*> sdfData::getObjectProperties() const
{
    return objectProperties;
}

void sdfData::setItemConstr(sdfData *constr)
{
    //constr->setLabel(this->getLabel() + "-items");
    // TODO: properties should also have this as their parent and not
    // the constraint
    constr->setParentCommon((sdfCommon*)this);
    item_constr = constr;
}

void sdfData::setConstantObject(sdfData *object)
{
    constantObject = object;
    constDefined = true;
}

void sdfData::setDefaultObject(sdfData *object)
{
    defaultObject = object;
    defaultDefined = true;
}

sdfData* sdfData::getConstantObject() const
{
    return constantObject;
}

sdfData* sdfData::getDefaultObject() const
{
    return defaultObject;
}

void sdfData::setObjectProperties(std::vector<sdfData*> properties)
{
    objectProperties = properties;
}
