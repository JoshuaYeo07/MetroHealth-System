#ifndef TABLE_PRINTER_HPP
#define TABLE_PRINTER_HPP

#include "PatientArray.hpp"
#include <iostream>
#include <iomanip>
#include <string>
#include "Console.hpp"

/**
 * @class TablePrinter
 * @brief Utilities to render formatted, aligned text-based console tables.
 * Complies with assignment guideline: "Do not use GUI, organize data neatly in text tables using formatting".
 */
class TablePrinter {
public:
    static void printHorizontalLine(int width = 110, char border = '-', char cross = '+') {
        std::cout << Console::DIM << cross;
        for (int i = 0; i < width - 2; ++i) {
            std::cout << border;
        }
        std::cout << cross << Console::RESET << "\n";
    }

    /**
     * @brief Prints a paginated or bounded table of Patient records.
     * @param arr The PatientArray to print
     * @param title Title of the dataset or query
     * @param maxRows Maximum rows to display (0 for all rows)
     */
    static void displayPatientTable(const PatientArray& arr, const std::string& title, size_t maxRows = 15) {
        const std::string rule(110, '=');
        std::cout << "\n" << Console::DIM << rule << Console::RESET << "\n";
        std::cout << Console::BOLD << Console::CYAN << "  " << title << " (Total Records: " << arr.size() << ")" << Console::RESET << "\n";
        std::cout << Console::DIM << rule << Console::RESET << "\n";

        if (arr.isEmpty()) {
            std::cout << Console::YELLOW << "  [Notice] No records found." << Console::RESET << "\n";
            std::cout << "--------------------------------------------------------------------------------------------------------------\n";
            return;
        }

        printHorizontalLine();
        std::cout << Console::BOLD << Console::YELLOW << "| " << std::left  << std::setw(12) << "Patient ID"
                  << "| " << std::right << std::setw(5)  << "Age"
                  << " | " << std::left  << std::setw(18) << "Care Type"
                  << "| " << std::right << std::setw(10) << "Stay (hrs)"
                  << "| " << std::right << std::setw(14) << "Rate (MYR/hr)"
                  << "| " << std::right << std::setw(11) << "Visits/Yr"
                  << "| " << std::right << std::setw(17) << "Total Cost (MYR)"
                  << " |" << Console::RESET << "\n";
        printHorizontalLine();

        size_t limit = (maxRows > 0 && maxRows < arr.size()) ? maxRows : arr.size();

        for (size_t i = 0; i < limit; ++i) {
            const Patient& p = arr[i];
            std::cout << "| " << std::left  << std::setw(12) << p.patientId
                      << "| " << std::right << std::setw(5)  << p.age
                      << " | " << std::left  << std::setw(18) << p.careType
                      << "| " << std::right << std::setw(10) << p.lengthOfStay
                      << "| " << std::right << std::setw(14) << std::fixed << std::setprecision(2) << p.baseCostPerHour
                      << "| " << std::right << std::setw(11) << p.daysVisitsPerYear
                      << "| " << std::right << std::setw(17) << std::fixed << std::setprecision(2) << p.calculateTotalCost()
                      << " |\n";
        }
        printHorizontalLine();

        if (maxRows > 0 && maxRows < arr.size()) {
            std::cout << "  * Displaying first " << limit << " of " << arr.size() 
                      << " records. (Full dataset safely stored in memory).\n";
        }
    }
};

#endif // TABLE_PRINTER_HPP
