/*
  strat150 Generic rules class
*/

#ifndef RULES_H_INCLUDED
#define RULES_H_INCLUDED

#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <list>
#include <set>
#include <iostream>
#include <fstream>
#include "Jzon.h"

/*! \file */
using namespace std;
using namespace Jzon;
namespace Rules
{
    /*!
        \class RulesEntry
        \brief Data structure for individual rules entries.
        \author Chris Livingston
    */
    class RulesEntry
    {
        set<string> doubleKeys;
        map <string, double> doubleVals;

        set<string> doubleArrayKeys;
        map <string, vector<double> > doubleArrays;

        set<string> stringKeys;
        map <string, string> stringVals;

        set<string> stringArrayKeys;
        map <string, vector<string> > stringArrays;


        public:

        string title;


        //Function prototypes

        string serialize();
        double getDouble(string key);
        vector<double> getDoubleArray(string key);
        string getString(string key);
        vector<string> getStringArray(string key);
        set<string> getDoubleKeys();
        set<string> getDoubleArrayKeys();
        set<string> getStringKeys();
        set<string> getStringArrayKeys();
        void addDouble(string key, double value);
        void rmDouble(string key);
        void addDoubleArray(string key, vector <double> value);
        void rmDoubleArray(string key);
        void appendDoubleArray(string key, double value);
        void addString(string key, string value);
        void rmString(string key);
        void addStringArray(string key, vector <string> value);
        void rmStringArray(string key);
        void appendStringArray(string key, string value);


        /*!
            \fn void parseJzon(string tag, Jzon::Object &entryNode)
            \brief Parses a Jzon object into a complete entry.
        */
        void parseJzon(string tag, Jzon::Object &entryNode)
        {
            title = tag;

            //if(entryNode.IsObject())
            //    entryNode = entryNode.Get(title).AsObject();

            ////cout << "Object retrieved.";

            for (Jzon::Object::iterator iter = entryNode.begin(); iter != entryNode.end(); iter++)
            {

                string userID = (*iter).first;
                Jzon::Node &node = (*iter).second;

                if (node.IsValue()) //Single
                {
                    if(node.IsNumber())
                        addDouble(userID, node.ToDouble());
                    else
                        addString(userID, node.ToString());
                }
                else if (node.IsArray()) //Array
                {
                    Jzon::Array arr = node.AsArray();

                    for(Jzon::Array::iterator aIter = arr.begin(); aIter != arr.end();aIter++)
                        if((*aIter).IsNumber())
                            appendDoubleArray(userID, (*aIter).ToDouble());
                        else
                            appendStringArray(userID, (*aIter).ToString());

                }
				else if (node.IsObject()){}

            }

        }


    };

    /*!
        \class RulesContainer
        \brief Parent container for game rules.
        \author Chris Livingston

        Contains two levels of RulesEntry classes, which contain
        single and array strings and doubles.  Can serialize to
        and from JSON.
    */
    class RulesContainer
    {
        private:
        set<string> singleEntryKeys;
        map <string, RulesEntry*> singleEntries;

        set<string> groupKeys;
        map <string, map<string, RulesEntry*>* > groupedEntries;

        public:


        ~RulesContainer()
        {
            map<string, RulesEntry*>::iterator entry = singleEntries.begin();

            for(; entry != singleEntries.end(); entry++)
                delete entry->second;

            //Grouped entries
            map<string, map<string, RulesEntry*>*>::iterator groupedEntry = groupedEntries.begin();
           for(; groupedEntry != groupedEntries.end(); groupedEntry++)
                {
                    //Delete entries
                    for(entry = groupedEntry->second->begin(); entry != groupedEntry->second->end(); entry++)
                        delete entry->second;

                    //Delete groups
                    delete groupedEntry->second;
                }
        }

        //Prototypes - Commented implementations are in rules.cpp

        RulesEntry* getEntry(string groupKey, string entryKey);
        set<string> getSingleEntryKeys();
        set<string> getGroupEntryKeys(string groupKey);
        set<string> getGroupKeys();
        bool rmEntry(string groupKey, string entryKey);
        string serialize();
        double getDouble(string groupKey, string entryKey, string valueKey);
        double getDouble(string entryKey, string valueKey);
        void addDouble(double value, string groupKey, string entryKey, string valueKey);
        vector<double> getDoubleArray(string groupKey, string entryKey, string valueKey);
        vector<double> getDoubleArray(string entryKey, string valueKey);
        void addDoubleArray(vector<double> value, string groupKey, string entryKey, string valueKey);
        string getString(string groupKey, string entryKey, string valueKey);
        string getString(string entryKey, string valueKey);
        void addString(string value, string groupKey, string entryKey, string valueKey);
        vector<string> getStringArray(string groupKey, string entryKey, string valueKey);
        vector<string> getStringArray(string entryKey, string valueKey);
        void addStringArray(vector<string> value, string groupKey, string entryKey, string valueKey);


       /*!
            \fn void addEntry(RulesEntry *entry, string groupKey)
            \param groupKey (optional) Target group
            \param entry Pointer to RulesEntry class to insert.
            \warning RulesEntry::title is used as the entry key.  Will replace RulesEntry class of matching key and group.
        */
        void addEntry(RulesEntry *entry, string groupKey = "")
        {
            //Single entry
            if(groupKey.length() == 0)
            {
                //Erase duplicate entry
                if(singleEntries.count(entry->title) == 1)
                {
                    delete singleEntries[entry->title];
                    singleEntries.erase(entry->title);
                }

                singleEntries.insert(std::make_pair(entry->title, entry));
                singleEntryKeys.insert(entry->title);
                return;
            }

            //Grouped entry - allocate group as needed
            if(groupedEntries.count(groupKey) == 0)
            {
                groupedEntries[groupKey] = new map<string, RulesEntry*>;
                groupKeys.insert(groupKey);
            }

            //Erase duplicate entry
            if(groupedEntries[groupKey]->count(entry->title) == 1)
            {
                delete groupedEntries[groupKey]->at(entry->title);
                groupedEntries[groupKey]->erase(entry->title);
            }

            groupedEntries[groupKey]->insert(std::make_pair(entry->title, entry));
        }


       /*!
            \fn void parseJzon(Jzon::Object &rootNode)
            \brief Parses a Jzon object and creates entry containers.
            \warning Use the deserialize() or loadJsonFile() functions instead.
        */
        void parseJzon(Jzon::Object &rootNode)
        {
            //All single entries
            for (Jzon::Object::iterator iter = rootNode.begin(); iter != rootNode.end(); iter++)
            {
                bool skip = false; //This value is set true if a nested object is found
                string userID = (*iter).first;
                Jzon::Node &node = (*iter).second;

                //If this node contains an object, it's a group and not a single entry
                if (node.IsObject())
                for (Jzon::Object::iterator iterEntry = node.AsObject().begin(); iterEntry != node.AsObject().end(); iterEntry++)
                {
                    //cout << "Checking " << name << endl;

                    if((*iterEntry).second.IsObject())
                     {
                         skip = true;
                         break;
                     }
                }
                else skip = true;

                //If this is a single entry, deserialize it
                if(!skip)
                {
                    //cout << "Deserializing " << name << "...";
                    RulesEntry *entry = new RulesEntry;
                    entry->parseJzon(userID, node.AsObject());

                    //cout << "done.  Adding..." << name << endl;
                    addEntry(entry);
                }

            }
            //cout << endl << "Done with singles..." << endl;

            //All group entries
            for (Jzon::Object::iterator iter = rootNode.begin(); iter != rootNode.end(); iter++)
            {
                string userID = (*iter).first;
                Jzon::Node &node = (*iter).second;

                //cout << "Checking " << name << endl;

                for (Jzon::Object::iterator iterEntry = node.AsObject().begin(); iterEntry != node.AsObject().end(); iterEntry++)
                {
                    //Inner object loop
                    if ((*iterEntry).second.IsObject())
                    {
                        RulesEntry *entry = new RulesEntry;

                        string entryKey = (*iterEntry).first;
                        Jzon::Node &entryNode = (*iterEntry).second;

                        entry->parseJzon(entryKey, entryNode.AsObject());

                        //name is the group key here
                        addEntry(entry, userID);
                    }
                }

            }
        }

       /*!
            \fn bool deserialize(string json)
            \brief Populates the object from a JSON string.
            \param json JSON data in STL string form.
            \return True on success, false on parsing error.  Error output is directed to stderr.
        */
        bool deserialize(string json)
        {
            Jzon::Object root;
            Jzon::Parser parser(root, json);

            if (!parser.Parse())
            {
                std::cerr << "Error: " << parser.GetError() << std::endl;
                return false;
            }
            else
            {
                RulesContainer testEntry;
                parseJzon(root);
            }

            return true;
        }

       /*!
            \fn bool loadJsonFile(string filename)
            \brief Populates the object from a JSON file.
            \param filename JSON file to parse.
            \return True on success, false on parsing error.  Error output is directed to stderr.
        */
        bool loadJsonFile(string filename)
        {
            ifstream t(filename.c_str());
            stringstream buffer;
            buffer << t.rdbuf();

            return deserialize(buffer.str());
        }



    };

};

#endif // RULES_H_INCLUDED
