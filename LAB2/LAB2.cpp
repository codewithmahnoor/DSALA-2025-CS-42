#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;


class Column {
public:
    string name;
    string type;
    unsigned int constrains;

    Column() {
        name = "";
        type = "";
        constrains = 0;
    }
};


class Row {
public:
    vector<string> value;
    Row(const vector<string>& val) : value(val) {}
};
class Table {
private:
    string tableName;
    vector<Column> columns;
    vector<Row*> rows;

public:
    Table(string name) : tableName(name) {}

    ~Table() {
        for (Row* r : rows)
            delete r;
        rows.clear();
    }

    void addColumn(string name, string type, unsigned int constrains) {
        Column c;
        c.name = name;
        c.type = type;
        c.constrains = constrains;
        columns.push_back(c);
    }

    void insertRow(vector<string> values) {
        rows.push_back(new Row(values));
        cout << "Record inserted.\n";
    }

    void selectAll() {
        for (Column& c : columns)
            cout << c.name << "\t";
        cout << "\n";

        for (Row* r : rows) {
            for (string val : r->value)
                cout << val << "\t";
            cout << "\n";
        }
    }

    void saveToFile() {
        ofstream file(tableName + ".txt");
        file << "TABLE " << tableName << "\n";

        for (Column& c : columns)
            file << c.name << " " << c.type << " " << c.constrains << "\n";

        file << "DATA\n";

        for (Row* r : rows) {
            for (string val : r->value)
                file << val << " ";
            file << "\n";
        }

        file.close();
        cout << "Saved to file.\n";
    }

    void loadFromFile() {
        ifstream file(tableName + ".txt");
        if (!file) { cout << "File not found.\n"; return; }

        string word;
        file >> word >> tableName;

        columns.clear();
        rows.clear();

        string colName, type;
        unsigned int cons;

        while (file >> colName) {
            if (colName == "DATA") break;
            file >> type >> cons;
            Column c;
            c.name = colName;
            c.type = type;
            c.constrains = cons;
            columns.push_back(c);
        }

        string line;
        getline(file, line); // skip to next line

        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            vector<string> values;
            string val;
            while (ss >> val)
                values.push_back(val);
            rows.push_back(new Row(values));
        }

        file.close();
        cout << "Loaded from file.\n";
    }
};




int main() {
    Table t("users");
    t.addColumn("id", "int", 1 | 2);
    t.addColumn("name", "string", 2);
    t.addColumn("age", "int", 0);

    t.insertRow({ "1001", "Ali", "15" });
    t.insertRow({ "1002", "mano", "20" });

    t.selectAll();

    return 0;
}