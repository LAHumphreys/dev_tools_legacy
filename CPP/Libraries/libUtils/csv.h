#ifndef DEV_TOOLS_CSV_H
#define DEV_TOOLS_CSV_H

#include<boost/tokenizer.hpp>
#include <string>
#include <sstream>
#include <cstring>
#include <array>
#include <vector>
#include "binaryReader.h"
#include "binaryWriter.h"
#include <type_traits>

#define TYPE(i) CSV<Types...>::ColTypes<i>
#define THIS CSV<Types...>

typedef boost::tokenizer<boost::escaped_list_separator<char> > Tokeniser;

//forward declare
class CSV_Column_Base;
class CSV_Cell;
template<class T>
class CSV_CellInstance;
template<class T>
class CSV_Column;


// TODO: Write file
template<class...Types>
class CSV {
private:
    using colsType = std::tuple<CSV_Column<Types>...>;
    using forwardRowType = std::tuple<Types&&...>;
public:
    // Type Info
    using rowType = std::tuple<Types&...>;
    template<std::size_t N>
    struct ColTypes {
        using type = typename std::tuple_element<N, colsType>;
    };
    static const int ncols = sizeof...(Types);
    int Rows();

    // Empty file
    CSV(){}

    // Move constructor
    CSV(CSV&& src) {
        std::swap(this->columns, src.columns);
    }

    // Get a Row
    rowType GetRow(int i);
    void GetRow(int i, Types&...args);

    // Remove a Row
    void RemoveRow(int i);

    // Add a Row
    void AddRow(Types&&...args);

    // Print a Row
    std::string PrintRow(int i);

    // Get a Column
    template <int i> 
    inline CSV_Column<ColTypes<i> >& GetColumn() { 
        return std::get<i>(columns);
    }

    // Get a cell
    template<int col>
    inline ColTypes<col>& GetCell(int row) {
        return std::get<col>(columns)[row];
    }

    static CSV<Types...> LoadCSV(BinaryReader reader);
    void WriteCSV(BinaryWriter writer);

private:

    // Utility Functions
    void NewRow(const Tokeniser& tok);

    // Data
    colsType columns;

    // Intenal Template Utilities

    //*********************************
    // template to add from tokens
    //*********************************
    template<int index>
    inline typename std::enable_if< index!=0,void>::type
    AddCell(Tokeniser::iterator& it);

    template<int index>
    inline typename std::enable_if< index==0,void>::type
    AddCell(Tokeniser::iterator& it);
    //*********************************
      
    //*********************************
    // template to add from values
    //*********************************
    template<int index>
    inline typename std::enable_if< index!=0,void>::type
    AddCell(forwardRowType&& row);

    template<int index>
    inline typename std::enable_if< index==0,void>::type
    AddCell(forwardRowType&& row);
    //*********************************

    //*********************************
    // template to get a row
    //*********************************

    // Used to by-pass the "must initialise reference" rule
    // WARNING - This is inherently dangerous, use with EXTREME CAUTION
    template<class T>
    class pT {
        public:
        inline operator T&() {  return *i; }
        inline pT& operator=(T& rhs) { i = &rhs; }
        T *i;
    };

    template< int index>
    inline typename std::enable_if< index!=0,void>::type
    GetCell(std::tuple<pT<Types>...>& t, int row);

    template< int index>
    inline typename std::enable_if< index==0,void>::type
    GetCell(std::tuple<pT<Types>...>& t, int row);
    //*********************************
      
    //*********************************
    // template to print a row
    //*********************************
    template<int index>
    inline typename std::enable_if< index!=0,void>::type
    PrintCell(std::stringstream& s, int row);

    template<int index>
    inline typename std::enable_if< index==0,void>::type
    PrintCell(std::stringstream& s, int row);
    //*********************************
      
    //*********************************
    // template to remove a row
    //*********************************
    template< int index>
    inline typename std::enable_if< index!=0,void>::type
    RemoveCell(int row);

    template< int index>
    inline typename std::enable_if< index==0,void>::type
    RemoveCell(int row);
    //*********************************
};

template<class T>
class CSV_Column_Common {
public:
    typedef T DataType;

    // Raw data acccess
    T* array() { return data.data(); }

    // Element access
    T& operator[](int i) { return data[i]; }
    const T& operator[](int i) const { return data[i]; }

    // Remove an element
    void remove(int i) {
        data.erase(data.begin() + i);
    }

    // Insert an element
    void emplace_back(T&& v) {
        data.emplace_back(v);
    }

    int size() {
        return data.size();
    }

    //
    // Enable itereation / range based for loops
    //
    inline typename std::vector<T>::iterator begin() { 
        return data.begin();
    }
    inline typename std::vector<T>::iterator end() { 
        return data.end();
    }
    inline typename std::vector<T>::const_iterator begin() const { 
        return data.cbegin();
    }
    inline typename std::vector<T>::const_iterator end() const { 
        return data.cend();
    }
protected:
    std::vector<T>  data;
};


template<class T>
class CSV_Column: public CSV_Column_Common<T> {
public:
    inline void NewRow(const string& token) {
        this->data.emplace_back(token);
    }

    inline void Print(std::stringstream& str, int row) {
        str << this->data[row];
    }
};

template<>
class CSV_Column<string>: public CSV_Column_Common<string> {
public:
    inline void NewRow(const string& token) {
        this->data.emplace_back(token);
    }

    inline void Print(std::stringstream& str, int row) {
        str << "\"";
        for(int i=0; i<data[row].length(); i++ ) {
            const char& c = data[row][i];
            if ( c == '"' ) {
                str << '\\' << '"';
            } else if ( c == '\n') {
                str << '\\' << 'n';
            } else {
                str << c;
            }
        }
        str << "\"";
    }
};

template<>
class CSV_Column<double>: public CSV_Column_Common<double> {
public:
    inline void NewRow(const string& token) {
        data.emplace_back(atof(token.c_str()));
    }

    inline void Print(std::stringstream& str, int row) {
        str << data[row];
    }
};

template<>
class CSV_Column<float>: public CSV_Column_Common<float> {
public:
    inline void NewRow(const string& token) {
        data.emplace_back(atof(token.c_str()));
    }

    inline void Print(std::stringstream& str, int row) {
        str << data[row];
    }
};

template<>
class CSV_Column<int>: public CSV_Column_Common<int> {
public:
    inline void NewRow(const string& token) {
        data.emplace_back(atoi(token.c_str()));
    }

    inline void Print(std::stringstream& str, int row) {
        str << data[row];
    }
};

template<>
class CSV_Column<long>: public CSV_Column_Common<long> {
public:
    void NewRow(const string& token) {
        data.emplace_back(atol(token.c_str()));
    }

    inline void Print(std::stringstream& str, int row) {
        str << data[row];
    }
};


#include "csv.hpp"

#endif
