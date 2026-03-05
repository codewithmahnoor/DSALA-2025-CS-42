#include <iostream>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

const unsigned int PRIMARY_KEY = 1;
const unsigned int NOT_NULL = 2;
const unsigned int UNIQUE = 4;

class Column {
    string name;
    string type;
    unsigned int constraints;

public:
    Column(string n, string t, unsigned int constr) {
        name = n;
        type = t;
        constraints = constr;
    }

    string getName() { return name; }
    string getType() { return type; }
    unsigned int getConstraints() { return constraints; }

    bool isPrimaryKey() { return (constraints & PRIMARY_KEY) != 0; }
    bool isNotNull() { return (constraints & NOT_NULL) != 0; }
    bool isUnique() { return (constraints & UNIQUE) != 0; }
};

class Row {
    vector<string> values;

public:
    Row(vector<string> vec) {
        values = vec;
    }

    vector<string> getValues() { return values; }

    string getValue(int index) {
        if (index >= 0 && index < (int)values.size())
            return values[index];
        return "";
    }
    int getSize() { return values.size(); }
};

class Table {
    string tableName;
    vector<Column> columns;
    vector<Row*> rows;

public:
    Table(string name, vector<Column> col) {
        tableName = name;
        columns = col;
    }

    ~Table() {
        // Crucial: Prevents memory leaks by freeing dynamically allocated rows
        for (int i = 0; i < (int)rows.size(); i++) {
            delete rows[i];
        }
        rows.clear();
    }

    void addRow(Row* r) {
        rows.push_back(r);
    }

    string getTableName() { return tableName; }
    vector<Column>& getColumns() { return columns; }
    vector<Row*>& getRows() { return rows; }
    int getColumnCount() { return columns.size(); }
    int getRowCount() { return rows.size(); }
};

Table* createTable() {
    char tableNameBuffer[256];
    int numCols;
    
    cout << "CREATE TABLE (Enter table name): ";
    cin >> tableNameBuffer;
    cout << "Enter number of columns: ";
    cin >> numCols;

    vector<Column> tempColumns;

    for (int i = 0; i < numCols; i++) {
        char colName[256];
        char colType[256];
        unsigned int constraints = 0;

        cout << "Column " << i + 1 << " name: ";
        cin >> colName;
        cout << "Type (int/string): ";
        cin >> colType;
        cout << "Constraints (0=None, 1=PK, 2=NotNull, 4=Unique, e.g. 3=PK+NotNull): ";
        cin >> constraints;

        tempColumns.push_back(Column(colName, colType, constraints));
    }
    
    Table* newTable = new Table(tableNameBuffer, tempColumns);
    cout << "Table created successfully.\n\n";
    return newTable;
}

void insertIntoTable(Table* t) {
    if (!t) return;
    
    vector<string> rowData;
    cout << "INSERT INTO " << t->getTableName() << " VALUES:\n";
    
    for (int i = 0; i < t->getColumnCount(); i++) {
        char valBuffer[256];
        Column c = t->getColumns()[i];
        
        cout << c.getName() << " (" << c.getType() << "): ";
        cin >> valBuffer;
        string valStr(valBuffer);

        // Manual validation using bitwise operators
        if (c.isNotNull() && valStr == "null") {
            cout << "Error: " << c.getName() << " cannot be null.\n";
            return;
        }
        
        if (c.isPrimaryKey() || c.isUnique()) {
            for (Row* r : t->getRows()) {
                if (r->getValue(i) == valStr) {
                    cout << "Error: Constraint violation. " << valStr << " already exists in " << c.getName() << ".\n";
                    return;
                }
            }
        }
        rowData.push_back(valStr);
    }
    
    t->addRow(new Row(rowData));
    cout << "Record inserted.\n\n";
}

void selectFromTable(Table* t) {
    if (!t) return;
    
    cout << "SELECT * FROM " << t->getTableName() << "\n";
    
    for (Column c : t->getColumns()) {
        cout << c.getName() << "\t";
    }
    cout << "\n-----------------------------------\n";
    
    for (Row* r : t->getRows()) {
        for (string val : r->getValues()) {
            cout << val << "\t";
        }
        cout << "\n";
    }
    cout << "\n";
}

void saveToFile(Table* t) {
    if (!t) return;
    
    string filename = t->getTableName() + ".txt";
    ofstream outFile(filename);
    
    if (!outFile) {
        cout << "Error opening file for saving.\n";
        return;
    }

    // Required File Format Structure
    outFile << "TABLE " << t->getTableName() << "\n";
    for (Column c : t->getColumns()) {
        outFile << c.getName() << " " << c.getType() << " " << c.getConstraints() << "\n";
    }
    
    outFile << "DATA\n";
    for (Row* r : t->getRows()) {
        for (int i = 0; i < r->getSize(); i++) {
            outFile << r->getValue(i) << (i == r->getSize() - 1 ? "" : " ");
        }
        outFile << "\n";
    }
    
    outFile.close();
    cout << "Saved to " << filename << " successfully.\n\n";
}

Table* loadFromFile() {
    char filenameBuffer[256];
    cout << "Enter filename to load (e.g., users.txt): ";
    cin >> filenameBuffer;
    
    ifstream inFile(filenameBuffer);
    if (!inFile) {
        cout << "File not found.\n";
        return nullptr;
    }

    string keyword, tableName;
    inFile >> keyword >> tableName; // Reads "TABLE users"

    vector<Column> cols;
    string colName, colType;
    unsigned int colConst;
    
    // Parses columns until it hits the "DATA" marker
    while (inFile >> colName && colName != "DATA") {
        inFile >> colType >> colConst;
        cols.push_back(Column(colName, colType, colConst));
    }

    Table* loadedTable = new Table(tableName, cols);

    // Parses the row values based on column count
    while (!inFile.eof()) {
        vector<string> rowVals;
        string val;
        for (int i = 0; i < (int)cols.size(); i++) {
            if (inFile >> val) {
                rowVals.push_back(val);
            }
        }
        if (rowVals.size() == cols.size()) {
            loadedTable->addRow(new Row(rowVals));
        }
    }

    inFile.close();
    cout << "Loaded successfully.\n\n";
    return loadedTable;
}

int main() {
    int choice = 0;
    Table* currentTable = nullptr;

    while (choice != 6) {
        cout << "1. Create Table\n2. Insert Row\n3. Select All\n4. Save to File\n5. Load from File\n6. Exit\nChoice: ";
        cin >> choice;
        cout << "\n";

        if (choice == 1) {
            if (currentTable) delete currentTable; // Prevent leaks if overriding
            currentTable = createTable();
        } 
        else if (choice == 2) {
            if (currentTable) insertIntoTable(currentTable);
            else cout << "Create or load a table first!\n\n";
        } 
        else if (choice == 3) {
            if (currentTable) selectFromTable(currentTable);
            else cout << "Create or load a table first!\n\n";
        } 
        else if (choice == 4) {
            saveToFile(currentTable);
        } 
        else if (choice == 5) {
            if (currentTable) delete currentTable;
            currentTable = loadFromFile();
        }
    }

    if (currentTable) {
        delete currentTable; // Final memory cleanup
    }
    
    return 0;
}