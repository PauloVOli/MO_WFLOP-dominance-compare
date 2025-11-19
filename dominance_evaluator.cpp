#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

namespace fs = std::filesystem;
using namespace std;

// Estrutura para armazenar soluções (fitness1, fitness2)
struct Solution {
    double f1;
    double f2;
};

// Função para ler soluções
vector<Solution> readSolutions(const string& filepath) {
    vector<Solution> solutions;
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "Erro ao abrir arquivo: " << filepath << endl;
        return solutions;
    }

    double f1, f2;
    while (file >> f1 >> f2)
        solutions.push_back({f1, f2});

    return solutions;
}

// Dominância de Pareto
bool dominates(const Solution &A, const Solution &B) {
    bool noWorse = (A.f1 <= B.f1 && A.f2 >= B.f2);
    bool strictlyBetter = (A.f1 < B.f1 || A.f2 > B.f2);
    return noWorse && strictlyBetter;
}

// Incomparabilidade
bool incomparable(const Solution &A, const Solution &B) {
    return !dominates(A, B) && !dominates(B, A);
}

// Conta dominâncias
int countDominating(const vector<Solution>& A, const vector<Solution>& B) {
    int count = 0;
    for (const auto& sa : A) {
        for (const auto& sb : B) {
            if (dominates(sa, sb)) {
                count++;
                break;
            }
        }
    }
    return count;
}

// Conta incomparáveis
int countIncomparable(const vector<Solution>& A, const vector<Solution>& B) {
    int count = 0;
    for (const auto& sa : A) {
        for (const auto& sb : B) {
            if (incomparable(sa, sb)) {
                count++;
                break;
            }
        }
    }
    return count;
}

// Função que encontra o arquivo EXATO baseado na instância
string findExactSolutionFile(const string& dir, const string& instance, const string& suffix) {
    string target = instance + suffix; // exemplo: "501_brkga_1000000.txt"

    for (const auto& f : fs::directory_iterator(dir)) {
        string name = f.path().filename().string();
        if (name == target)
            return f.path().string();
    }

    return "";
}

// MAIN
int main() {
    string algA_path = "BRKGA";
    string algB_path = "MOEAD";
    string output_csv = "dominance_results.csv";

    ofstream out(output_csv);
    out << "Instance,Run,BRKGA_Dominates,MOEAD_Dominates,"
           "BRKGA_Incomparable,MOEAD_Incomparable,TotalBRKGA,TotalMOEAD\n";

    for (const auto& inst_dir : fs::directory_iterator(algA_path)) {
        string instance = inst_dir.path().filename().string();

        string inst_path_B = algB_path + "/" + instance;
        if (!fs::exists(inst_path_B))
            continue;

        for (const auto& run_dir : fs::directory_iterator(inst_dir.path())) {
            string run = run_dir.path().filename().string();

            string run_path_B = inst_path_B + "/" + run;
            if (!fs::exists(run_path_B))
                continue;

            // Arquivos exatos que devem ser lidos
            string fileA = findExactSolutionFile(run_dir.path().string(), instance, "_brkga_1000000.txt");
            string fileB = findExactSolutionFile(run_path_B, instance, "_moead_1000000.txt");

            if (fileA.empty() || fileB.empty()) {
                cerr << "Arquivo não encontrado para " << instance << " run " << run << endl;
                continue;
            }

            vector<Solution> solA = readSolutions(fileA);
            vector<Solution> solB = readSolutions(fileB);

            if (solA.empty() || solB.empty()) {
                cerr << "Arquivo vazio em " << instance << " run " << run << endl;
                continue;
            }

            int domA = countDominating(solA, solB);
            int domB = countDominating(solB, solA);

            int incA = countIncomparable(solA, solB);
            int incB = countIncomparable(solB, solA);

            out << instance << "," << run << ","
                << domA << "," << domB << ","
                << incA << "," << incB << ","
                << solA.size() << "," << solB.size() << "\n";
        }
    }

    out.close();
    cout << "Comparação finalizada! Resultados salvos em: " << output_csv << endl;
    return 0;
}


