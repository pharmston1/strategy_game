#include "rules.h"

/*! \file */
namespace Rules
{


    /*!
        \fn RecordEntry* RulesContainer::getEntry(string groupKey, string entryKey)
        \brief Retrieves a RulesEntry class within.
        \param groupKey Group key.  Use a blank string ("") to retrieve from single entries.
        \param entryKey The key/title of the RulesEntry class containing the value.
        \return Pointer to RulesEntry class.
    */
    RulesEntry* RulesContainer::getEntry(string groupKey, string entryKey)
    {
       //Zero-length groupKey
        if(groupKey.size() == 0 && singleEntries.count(entryKey) == 1)
            return singleEntries[entryKey];

        if(groupedEntries.count(groupKey) == 1)
        {
            map<string, RulesEntry*>* m = groupedEntries[groupKey];

            if(m->count(entryKey) == 1)
                return m->at(entryKey);
        }

        return NULL; //not found
    }


    /*!
        \fn set<string> RulesContainer::getSingleEntryKeys()
        \return Set of this container's single entry keys.
    */
    set<string> RulesContainer::getSingleEntryKeys()
    {
        return singleEntryKeys;
    }

    /*!
        \fn set<string> RulesContainer::getGroupEntryKeys()
        \param groupKey Key to group to retrieve keys from.
        \return Set of entry keys from the specified group.
        \warning Will return an empty set if the groupKey is not found.
    */
    set<string> RulesContainer::getGroupEntryKeys(string groupKey)
    {
        set<string> s;

        if(groupedEntries.count(groupKey) == 0)
            return s;

        //Build the list
        //TODO:  Cache results
        map<string, RulesEntry*>::iterator entry = groupedEntries[groupKey]->begin();

        for(; entry != groupedEntries[groupKey]->end(); entry++)
            s.insert(entry->first);

        return s;
    }

    /*!
        \fn set<string> RulesContainer::getGroupKeys()
        \return Set of group keys in this container.
    */
    set<string> RulesContainer::getGroupKeys()
    {
        return groupKeys;
    }

    /*!
        \fn void RulesContainer::rmEntry(string groupKey, string entryKey)
        \brief Removes an entry.  Also removes newly-empty groups.
        \param groupKey Key to parent group of entry.  Leave blank ("") for single entries.
        \param entryKey Key to entry to remove.
        \return True on success.
    */
    bool RulesContainer::rmEntry(string groupKey, string entryKey)
    {
        RulesEntry *r;

        //For single entries
        if(groupKey.size() == 0 && singleEntries.count(entryKey) == 1)
        {
            r = singleEntries[entryKey];
            singleEntries.erase(entryKey);
            singleEntryKeys.erase(entryKey);
            delete r;
            return true;
        }

        //Entry is within a group
        if(groupedEntries.count(groupKey) == 1)
        {
            //Fail if not found
            if(groupedEntries[groupKey]->count(entryKey) == 0)
                return false;

            r = groupedEntries[groupKey]->at(entryKey);

            groupedEntries[groupKey]->erase(entryKey);
            delete r;

            //Cull the group if it's empty
            if(groupedEntries[groupKey]->size() == 0)
            {
                groupKeys.erase(groupKey);
                delete groupedEntries[groupKey];
                groupedEntries.erase(groupKey);
            }

            return true;
        }

        return false;
    }


    /*!
        \fn string RulesContainer::serialize()
        \brief Serializes all member RulesEntry classes in one big JSON string.
        \return This RulesContainer fully serialized in JSON.
     */
    string RulesContainer::serialize()
    {
        string s = "{\n\n";

        //Single entries
        map<string, RulesEntry*>::iterator entry = singleEntries.begin();
        for(; entry != singleEntries.end(); entry++)
            s += entry->second->serialize() + ",\n";

        //Grouped entries
        map<string, map<string, RulesEntry*>*>::iterator groupedEntry = groupedEntries.begin();
        for(; groupedEntry != groupedEntries.end(); groupedEntry++)
        {

            s += "\"" + groupedEntry->first + "\"" + " :\n{\n";

            int i = 0;
            for(entry = groupedEntry->second->begin(); entry != groupedEntry->second->end(); entry++)
            {
                 s += entry->second->serialize() + ",\n";
                 i++;
            }

            //Trim comma
            if(i > 0)
                s = s.substr(0, s.size()-2);
            s += "\n";

            s += "},\n";
        }

        //Trim comma
        s = s.substr(0, s.size()-2);

        s += "\n\n}";

        return s;
    }

    /*!
        \fn double RulesContainer::getDouble(string groupKey, string entryKey, string valueKey)
        \brief Gets a double value from a grouped or single rules entry.
        \param groupKey Group key.  Use a blank string ("") to retrieve from single entries.
        \param entryKey The key/title of the RulesEntry class containing the value.
        \param valueKey Key to value within target RulesEntry class.
        \return Value if found, 0 if entry, group or value are not found.
     */
    double RulesContainer::getDouble(string groupKey, string entryKey, string valueKey)
    {
        //Zero-length groupKey
        if(groupKey.size() == 0)
            return getDouble(entryKey, valueKey);


        if(groupedEntries.count(groupKey) == 1)
        {
            map<string, RulesEntry*>* m = groupedEntries[groupKey];

            if(m->count(entryKey) == 1)
                return m->at(entryKey)->getDouble(valueKey);
            else
                return 0; //entry not found
        }
        else return 0; //group not found
    }


   /*!
        \fn double RulesContainer::getDouble(string entryKey, string valueKey)
        \brief Gets a double value from a single rules entry only.
        \param entryKey The key/title of the RulesEntry class containing the value.
        \param valueKey Key to value within target RulesEntry class.
        \warning Cannot search grouped entries.
        \return Value if found, 0 if entry or value are not found.
    */
    double RulesContainer::getDouble(string entryKey, string valueKey)
    {
        //Check if entryKey matches any groups
        if(singleEntries.count(entryKey) == 1)
        {
            return singleEntries.at(entryKey)->getDouble(valueKey);
        }

        return 0;
    }

    /*!
        \fn void RulesContainer::addDouble(double value, string groupKey, string entryKey, string valueKey)
        \brief Adds a double record.  Will automatically create groups and entries as needed.
        \param groupKey Group key.  Use a blank string ("") to store to a single entry.
        \param entryKey The key/title of the RulesEntry class containing the value.
        \param valueKey Key to value within target RulesEntry class.
    */
    void RulesContainer::addDouble(double value, string groupKey, string entryKey, string valueKey)
    {
        //cout << "Adding double." << endl;
        //Single entry
        if(groupKey.size() == 0)
        {
            //Allocate a new entry if one doesn't already exist
            if(singleEntries.count(entryKey) == 0)
            {
                singleEntries[entryKey] = new RulesEntry;
                singleEntries[entryKey]->title = entryKey;
            }


            singleEntries[entryKey]->addDouble(valueKey, value);
        }
        else
        {
            //Add the group as needed
            if(groupedEntries.count(groupKey) == 0)
                groupedEntries[groupKey] = new map<string, RulesEntry*>;

            //Allocate a new entry if one doesn't already exist
            if(groupedEntries[groupKey]->count(entryKey) == 0)
            {
                RulesEntry *r = new RulesEntry;
                r->title = entryKey;
                groupedEntries[groupKey]->insert(std::make_pair(entryKey, r));
            }


            groupedEntries[groupKey]->at(entryKey)->addDouble(valueKey, value);


        }
    }

    ////////////////////////////////

     /*!
        \fn vector<double> RulesContainer::getDoubleArray(string groupKey, string entryKey, string valueKey)
        \brief Gets a double array (STL vector) value from a grouped or single rules entry.
        \param groupKey Group key.  Use a blank string ("") to retrieve from single entries.
        \param entryKey The key/title of the RulesEntry class containing the value.
        \param valueKey Key to value within target RulesEntry class.
        \return Value if found, empty vector if entry, group or value are not found.
     */
    vector<double> RulesContainer::getDoubleArray(string groupKey, string entryKey, string valueKey)
    {
        //Zero-length groupKey
        if(groupKey.size() == 0)
            return getDoubleArray(entryKey, valueKey);

        if(groupedEntries.count(groupKey) == 1)
        {
            map<string, RulesEntry*>* m = groupedEntries[groupKey];

            if(m->count(entryKey) == 1)
                return m->at(entryKey)->getDoubleArray(valueKey);
        }

        vector<double> v; //Zero-length dummy vector
        return v; //group or entry not found
    }

    /*!
        \fn vector<double> RulesContainer::getDoubleArray(string entryKey, string valueKey)
        \brief Gets a double array (STL vector) value from a single rules entry only.
        \param entryKey The key/title of the RulesEntry class containing the value.
        \param valueKey Key to value within target RulesEntry class.
        \warning Cannot search grouped entries.
        \return Value if found, empty vector if entry or value are not found.
    */
    vector<double> RulesContainer::getDoubleArray(string entryKey, string valueKey)
    {
        //Check if entryKey matches any groups
        if(singleEntries.count(entryKey) == 1)
        {
            return singleEntries.at(entryKey)->getDoubleArray(valueKey);
        }

        vector<double> v;
        return v;
    }

    /*!
        \fn void RulesContainer::addDoubleArray(vector<double> value, string groupKey, string entryKey, string valueKey)
        \brief Adds a double array (STL vector) record.  Will automatically create groups and entries as needed.
        \param groupKey Group key.  Use a blank string ("") to store to a single entry.
        \param entryKey The key/title of the RulesEntry class containing the value.
        \param valueKey Key to value within target RulesEntry class.
    */
    void RulesContainer::addDoubleArray(vector<double> value, string groupKey, string entryKey, string valueKey)
    {
        //Single entry
        if(groupKey.size() == 0)
        {
            //Allocate a new entry if one doesn't already exist
            if(singleEntries.count(entryKey) == 0)
            {
                singleEntries[entryKey] = new RulesEntry;
                singleEntries[entryKey]->title = entryKey;
            }

            singleEntries[entryKey]->addDoubleArray(valueKey, value);
        }
        else
        {
            //Add the group as needed
            if(groupedEntries.count(groupKey) == 0)
                groupedEntries[groupKey] = new map<string, RulesEntry*>;

            //Allocate a new entry if one doesn't already exist
            if(groupedEntries[groupKey]->count(entryKey) == 0)
            {
                RulesEntry *r = new RulesEntry;
                r->title = entryKey;
                groupedEntries[groupKey]->insert(std::make_pair(entryKey, r));
            }

            groupedEntries[groupKey]->at(entryKey)->addDoubleArray(valueKey, value);

        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////

    /*!
        \fn string RulesContainer::getString(string groupKey, string entryKey, string valueKey)
        \brief Gets a string value from a grouped or single rules entry.
        \param groupKey Group key.  Use a blank string ("") to retrieve from single entries.
        \param entryKey The key/title of the RulesEntry class containing the value.
        \param valueKey Key to value within target RulesEntry class.
        \return Value if found, blank ("") if entry, group or value are not found.
     */
    string RulesContainer::getString(string groupKey, string entryKey, string valueKey)
    {
        //Zero-length groupKey
        if(groupKey.size() == 0)
            return getString(entryKey, valueKey);


        if(groupedEntries.count(groupKey) == 1)
        {
            map<string, RulesEntry*>* m = groupedEntries[groupKey];

            if(m->count(entryKey) == 1)
                return m->at(entryKey)->getString(valueKey);
            else
                return ""; //entry not found
        }
        else return ""; //group not found
    }


   /*!
        \fn string RulesContainer::getString(string entryKey, string valueKey)
        \brief Gets a string value from a single rules entry only.
        \param entryKey The key/title of the RulesEntry class containing the value.
        \param valueKey Key to value within target RulesEntry class.
        \warning Cannot search grouped entries.
        \return Value if found, blank ("") if entry or value are not found.
    */
    string RulesContainer::getString(string entryKey, string valueKey)
    {
        //Check if entryKey matches any groups
        if(singleEntries.count(entryKey) == 1)
        {
            return singleEntries.at(entryKey)->getString(valueKey);
        }

        return "";
    }

    /*!
        \fn void RulesContainer::addString(string value, string groupKey, string entryKey, string valueKey)
        \brief Adds a string record.  Will automatically create groups and entries as needed.
        \param groupKey Group key.  Use a blank string ("") to store to a single entry.
        \param entryKey The key/title of the RulesEntry class containing the value.
        \param valueKey Key to value within target RulesEntry class.
    */
    void RulesContainer::addString(string value, string groupKey, string entryKey, string valueKey)
    {
        //Single entry
        if(groupKey.size() == 0)
        {

            //Allocate a new entry if one doesn't already exist
            if(singleEntries.count(entryKey) == 0)
            {
                singleEntries[entryKey] = new RulesEntry;
                singleEntries[entryKey]->title = entryKey;
            }

            singleEntries[entryKey]->addString(valueKey, value);
        }
        else
        {
            //Add the group as needed
            if(groupedEntries.count(groupKey) == 0)
                groupedEntries[groupKey] = new map<string, RulesEntry*>;

            //Allocate a new entry if one doesn't already exist
            if(groupedEntries[groupKey]->count(entryKey) == 0)
            {
                RulesEntry *r = new RulesEntry;
                r->title = entryKey;
                groupedEntries[groupKey]->insert(std::make_pair(entryKey, r));
            }

            groupedEntries[groupKey]->at(entryKey)->addString(valueKey, value);

        }
    }

    ////////////////////////////////

     /*!
        \fn vector<string> RulesContainer::getStringArray(string groupKey, string entryKey, string valueKey)
        \brief Gets a string array (STL vector) value from a grouped or single rules entry.
        \param groupKey Group key.  Use a blank string ("") to retrieve from single entries.
        \param entryKey The key/title of the RulesEntry class containing the value.
        \param valueKey Key to value within target RulesEntry class.
        \return Value if found, empty vector if entry, group or value are not found.
     */
    vector<string> RulesContainer::getStringArray(string groupKey, string entryKey, string valueKey)
    {
        //Zero-length groupKey
        if(groupKey.size() == 0)
            return getStringArray(entryKey, valueKey);

        if(groupedEntries.count(groupKey) == 1)
        {
            map<string, RulesEntry*>* m = groupedEntries[groupKey];

            if(m->count(entryKey) == 1)
                return m->at(entryKey)->getStringArray(valueKey);
        }

        vector<string> v; //Zero-length dummy vector
        return v; //group or entry not found
    }

    /*!
        \fn vector<string> RulesContainer::getStringArray(string entryKey, string valueKey)
        \brief Gets a string array (STL vector) value from a single rules entry only.
        \param entryKey The key/title of the RulesEntry class containing the value.
        \param valueKey Key to value within target RulesEntry class.
        \warning Cannot search grouped entries.
        \return Value if found, empty vector if entry or value are not found.
    */
    vector<string> RulesContainer::getStringArray(string entryKey, string valueKey)
    {
        //Check if entryKey matches any groups
        if(singleEntries.count(entryKey) == 1)
        {
            return singleEntries.at(entryKey)->getStringArray(valueKey);
        }

        vector<string> v;
        return v;
    }

    /*!
        \fn void RulesContainer::addStringArray(vector<string> value, string groupKey, string entryKey, string valueKey)
        \brief Adds a string array (STL vector) record.  Will automatically create groups and entries as needed.
        \param groupKey Group key.  Use a blank string ("") to store to a single entry.
        \param entryKey The key/title of the RulesEntry class containing the value.
        \param valueKey Key to value within target RulesEntry class.
    */
    void RulesContainer::addStringArray(vector<string> value, string groupKey, string entryKey, string valueKey)
    {
        //Single entry
        if(groupKey.size() == 0)
        {
            //Allocate a new entry if one doesn't already exist
            if(singleEntries.count(entryKey) == 0)
            {
                singleEntries[entryKey] = new RulesEntry;
                singleEntries[entryKey]->title = entryKey;
            }

            singleEntries[entryKey]->addStringArray(valueKey, value);
        }
        else
        {

            //Add the group as needed
            if(groupedEntries.count(groupKey) == 0)
                groupedEntries[groupKey] = new map<string, RulesEntry*>;

            //Allocate a new entry if one doesn't already exist
            if(groupedEntries[groupKey]->count(entryKey) == 0)
            {
                RulesEntry *r = new RulesEntry;
                r->title = entryKey;
                groupedEntries[groupKey]->insert(std::make_pair(entryKey, r));
            }

            groupedEntries[groupKey]->at(entryKey)->addStringArray(valueKey, value);

        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////

    /// Returns this object serialzed into JSON
    string RulesEntry::serialize()
    {
        //Opening
        string r = "  \"" + title + "\" :\n  {  \n";

        //First, the strings
        map<string, string>::iterator iterS;
        iterS = stringVals.begin();
        while(iterS != stringVals.end())
        {
            r += "    \"" + iterS->first + "\": " + "\"" + iterS->second + "\"";
            r += ",\n";
            iterS++;
        }

        //Next, the arrays of strings
        map<string, vector<string> >::iterator iterSA;
        iterSA = stringArrays.begin();
        while(iterSA != stringArrays.end())
        {
            //Header
            r += "    \"" + iterSA->first + "\": [";

            vector<string>::iterator elementSA = iterSA->second.begin();
            string q;
            //Cycle through each array element
            for(; elementSA != iterSA->second.end(); elementSA++)
                q += "\"" + *elementSA + "\", ";

            r += q.substr(0, q.size()-2);

            r += "],\n";
            iterSA++;
        }

        //Now the doubles - note use of stringstream to convert
        map<string, double>::iterator iterD;
        iterD = doubleVals.begin();
        while(iterD != doubleVals.end())
        {
            std::stringstream s;
            s << "    \"" << iterD->first << "\": " << iterD->second;
            s << ",\n";

            r += s.str();
            iterD++;
        }

        //Next, the arrays of doubles
        map<string, vector<double> >::iterator iterDA;
        iterDA = doubleArrays.begin();
        while(iterDA != doubleArrays.end())
        {
            //Header
            r += "    \"" + iterDA->first + "\": [";

            vector<double>::iterator elementDA = iterDA->second.begin();
            std::stringstream q;
            //Cycle through each array element
            for(; elementDA != iterDA->second.end(); elementDA++)
                q  << *elementDA << ", ";

            r += q.str().substr(0, q.str().size()-2); //remove last comma

            r += "],\n";
            iterDA++;
        }

        //Pop off last comma+newline
        r = r.substr(0, r.size()-2);
        r += "\n  }";


        return r;

    }

    /// Retrieves a double value
    double RulesEntry::getDouble(string key)
    {
        map<string, double>::iterator f = doubleVals.find(key);
        if(f != doubleVals.end())
            return f->second;

        return 0;
    }

    /// Retrieves an array (STL vector) of doubles
    vector<double> RulesEntry::getDoubleArray(string key)
    {
        map<string, vector<double> >::iterator f = doubleArrays.find(key);
        if(f != doubleArrays.end())
            return f->second;

        return vector<double>();
    }

    /// Retrieves a string
    string RulesEntry::getString(string key)
    {
        map<string, string>::iterator f = stringVals.find(key);
        if(f != stringVals.end())
            return f->second;

        return "";//stringVals.find(key)->second;
    }

    /// Retrieves an array (STL vector) of strings
    vector<string> RulesEntry::getStringArray(string key)
    {
        map<string, vector<string> >::iterator f = stringArrays.find(key);
        if(f != stringArrays.end())
            return f->second;

        return vector<string>();
    }

    /// Retrieves the list of double keys
    set<string> RulesEntry::getDoubleKeys()
    {
        return doubleKeys;
    }

    /// Retrieves the list of double array keys
    set<string> RulesEntry::getDoubleArrayKeys()
    {
        return doubleArrayKeys;
    }

    /// Retrieves the list of string keys
    set<string> RulesEntry::getStringKeys()
    {
        return stringKeys;
    }

    /// Retrieves the list of string array keys
    set<string> RulesEntry::getStringArrayKeys()
    {
        return stringArrayKeys;
    }

    /// Adds a double value - replaces existing values
    void RulesEntry::addDouble(string key, double value)
    {
        //Over-write the existing value
        if(doubleVals.count(key) == 1)
            doubleVals.erase(key);
        else
            doubleKeys.insert(key);

        doubleVals.insert(std::make_pair(key, value));

    }

    /// Removes a double value
    void RulesEntry::rmDouble(string key)
    {
        doubleVals.erase(key);
        doubleKeys.erase(key);
    }

    /// Adds an entire double array - replaces existing values
    void RulesEntry::addDoubleArray(string key, vector <double> value)
    {
        //Over-write the existing value
        if(doubleArrays.count(key) == 1)
            doubleArrays.erase(key);
        else
            doubleArrayKeys.insert(key);

        doubleArrays.insert(std::make_pair(key, value));

    }

    /// Removes a double array
    void RulesEntry::rmDoubleArray(string key)
    {
        doubleArrays.erase(key);
        doubleArrayKeys.erase(key);
    }

    /// Appends a value to a double array - creates a new array entry if one does not exist
    void RulesEntry::appendDoubleArray(string key, double value)
    {
        //Add a new array if it doesn't exist
        if(doubleArrays.count(key) == 0)
        {
            vector<double> v;
            v.push_back(value);

            addDoubleArray(key, v);
            return;
        }

        doubleArrays.find(key)->second.push_back(value);
    }

    /// Adds a string value - replaces existing values
    void RulesEntry::addString(string key, string value)
    {
        //Over-write the existing value
        if(stringVals.count(key) == 1)
            stringVals.erase(key);
        else
            stringKeys.insert(key);

        stringVals.insert(std::make_pair(key, value));

    }

    /// Removes a string value
    void RulesEntry::rmString(string key)
    {
        stringVals.erase(key);
        stringKeys.erase(key);
    }

    /// Adds an entire string array - replaces existing values
    void RulesEntry::addStringArray(string key, vector <string> value)
    {
        //Over-write the existing value
        if(stringArrays.count(key) == 1)
            stringArrays.erase(key);
        else
            stringArrayKeys.insert(key);

        stringArrays.insert(std::make_pair(key, value));

    }

    /// Removes a string array
    void RulesEntry::rmStringArray(string key)
    {
        stringArrays.erase(key);
        stringArrayKeys.erase(key);
    }

    /// Appends a value to a string array - creates a new array entry if one does not exist
    void RulesEntry::appendStringArray(string key, string value)
    {
        //Add a new array if it doesn't exist
        if(stringArrays.count(key) == 0)
        {
            vector<string> v;
            v.push_back(value);

            addStringArray(key, v);
            return;
        }

        stringArrays.find(key)->second.push_back(value);
    }

};
