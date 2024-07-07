//
// Created by zoravur on 6/22/24.
// The actual spreadsheet data structure.
//

#ifndef SPREADSHEETMODEL_HPP
#define SPREADSHEETMODEL_HPP
#include <memory>
#include <unordered_map>
#include <map>
#include <vector>
#include <fmt/format.h>

#include "Subject.hpp"

/*
struct Formula {
    virtual ~Formula() = default;
    virtual void set(const std::string& fmla) = 0;
    virtual std::string show() = 0;
};

struct String final : Formula {
    std::string value;
    std::string show() override {
        return value;
    }
    void set(const std::string& fmla) override {
        value = fmla;
    }
};

// struct DirectedNode {
//     std::vector<std::pair<int, int>> in_nodes;
//     std::vector<std::pair<int, int>> out_nodes;
// };

struct Cell //: public DirectedNode
{
    std::unique_ptr<Formula> formula;

    Cell() = default;


};*/

class SpreadsheetModel : public Subject<size_t, size_t, std::string> {
    // std::map<std::pair<int, int>, Cell> cells;
public:
    size_t n_rows = 100, n_cols = 26;
private:
    std::vector<std::vector<std::string>> cells; // store in column-major order


public:
    SpreadsheetModel(): n_rows(100), n_cols(26), cells(n_cols, std::vector<std::string>(n_rows, std::string())) {}


    std::string getCellValue(size_t col, size_t row) {
        if (row-1 >= n_rows || col-1 >= n_cols) {
            throw std::out_of_range( fmt::format("Cell out of range {} {}", (int)col, (int)row) );
        }
        return cells[col-1][row-1];
    }

    void setCellValue(size_t col, size_t row, const std::string& val) {
        if (row-1 >= n_rows || col-1 >= n_cols) {
            throw std::out_of_range( fmt::format("Cell out of range {} {} {}", (int)col, (int)row, val) );
        }
        cells[col-1][row-1] = val;
        notify(col, row, val);
    }

    [[nodiscard]] bool inBounds(size_t col, size_t row) const {
        return col >= 1 && row >= 1 && col - 1 < n_cols && row - 1 < n_rows;
    }

    // std::string getCellValue(std::pair<int,int> key) {
    //     return cells[key].formula->show();
    // }
    //
    // void setCellValue(std::pair<int, int> key, const std::string& value) {
    //     cells[key].formula->set(value);
    // }


};



#endif //SPREADSHEETMODEL_HPP
