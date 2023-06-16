#include <iostream>
#include <vector>

#include <TFile.h>
#include <TTree.h>

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " CAEN-V1742_output.root\n";
    return 1;
  };

  uint8_t  group;
  uint32_t timestamp;
  std::vector<std::vector<double>>* waveforms = nullptr;

  TFile file(argv[1], "READ");
  auto tree = dynamic_cast<TTree*>(file.Get("CAEN-V1742"));
  if (!tree) {
    std::cerr << "Tree `CAEN-V1742` is not found in `" << argv[1] << "'\n";
    return 1;
  };

  tree->SetBranchAddress("timestamp", &timestamp);
  tree->SetBranchAddress("waveforms", &waveforms);

  long n = tree->GetEntries();
  for (long e = 0; e < n; ++e) {
    tree->GetEntry(e);
    std::cout << "# event " << e << '\t' << timestamp << '\n';

    uint32_t sizes[32];
    uint32_t nsamples = 0;
    for (int c = 0; c < 32; ++c) {
      sizes[c] = (*waveforms)[c].size();
      if (sizes[c] > nsamples) nsamples = sizes[c];
    };

    for (uint32_t i = 0; i < nsamples; ++i) {
      for (int c = 0; c < 32; ++c) {
        if (c > 0) std::cout << '\t';
        if (i < sizes[c])
          std::cout << (*waveforms)[c][i];
        else
          std::cout << 0;
      };
      std::cout << '\n';
    };
    std::cout << '\n';
  };
  return 0;
};
