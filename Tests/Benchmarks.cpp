#include "PluginEditor.h"
#include "catch2/benchmark/catch_benchmark_all.hpp"
#include "catch2/catch_test_macros.hpp"

TEST_CASE ("Boot performance")
{
    BENCHMARK_ADVANCED ("Processor constructor")
    (Catch::Benchmark::Chronometer meter)
    {
        auto gui = juce::ScopedJuceInitialiser_GUI {};
        std::vector<Catch::Benchmark::storage_for<EmpyAudioProcessor>> storage (size_t (meter.runs()));
        meter.measure ([&] (int i) { storage[(size_t) i].construct(); });
    };

    BENCHMARK_ADVANCED ("Processor destructor")
    (Catch::Benchmark::Chronometer meter)
    {
        auto gui = juce::ScopedJuceInitialiser_GUI {};
        std::vector<Catch::Benchmark::destructable_object<EmpyAudioProcessor>> storage (size_t (meter.runs()));
        for (auto& s : storage)
            s.construct();
        meter.measure ([&] (int i) { storage[(size_t) i].destruct(); });
    };

    BENCHMARK_ADVANCED ("Editor open and close")
    (Catch::Benchmark::Chronometer meter)
    {
        auto gui = juce::ScopedJuceInitialiser_GUI {};

        EmpyAudioProcessor plugin;

        // due to complex construction logic of the editor, let's measure open/close together
        meter.measure ([&] (int i) {
            std::cout << "creating editor\n";
            auto editor = plugin.createEditorIfNeeded();
            std::cout << "to delete\n";
            plugin.editorBeingDeleted (editor);
            std::cout << "deleting editor\n";
            delete editor;
            std::cout << "get active editor\n";
            return plugin.getActiveEditor();
        });
    };
}
