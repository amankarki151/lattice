add_test([=[WalTest.ReplayOnEmptyFileReturnsNothing]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=WalTest.ReplayOnEmptyFileReturnsNothing]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[WalTest.ReplayOnEmptyFileReturnsNothing]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_wal.cpp:29]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[WalTest.SingleRecordRoundTrips]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=WalTest.SingleRecordRoundTrips]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[WalTest.SingleRecordRoundTrips]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_wal.cpp:34]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[WalTest.ManyRecordsReplayInOrder]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=WalTest.ManyRecordsReplayInOrder]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[WalTest.ManyRecordsReplayInOrder]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_wal.cpp:47]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[WalTest.TornFinalRecordIsDroppedButEarlierOnesSurvive]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=WalTest.TornFinalRecordIsDroppedButEarlierOnesSurvive]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[WalTest.TornFinalRecordIsDroppedButEarlierOnesSurvive]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_wal.cpp:64]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[WalTest.AppendsRatherThanOverwrites]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=WalTest.AppendsRatherThanOverwrites]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[WalTest.AppendsRatherThanOverwrites]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_wal.cpp:88]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[SegmentTest.MissingFileIsNotAnError]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=SegmentTest.MissingFileIsNotAnError]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[SegmentTest.MissingFileIsNotAnError]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_segment.cpp:30]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[SegmentTest.RoundTripsAllRecords]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=SegmentTest.RoundTripsAllRecords]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[SegmentTest.RoundTripsAllRecords]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_segment.cpp:35]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[SegmentTest.CorruptMagicIsRejected]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=SegmentTest.CorruptMagicIsRejected]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[SegmentTest.CorruptMagicIsRejected]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_segment.cpp:55]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[SegmentTest.EmptySegmentIsValid]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=SegmentTest.EmptySegmentIsValid]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[SegmentTest.EmptySegmentIsValid]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_segment.cpp:71]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[DistanceTest.IdenticalVectorsAreZeroApart]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=DistanceTest.IdenticalVectorsAreZeroApart]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[DistanceTest.IdenticalVectorsAreZeroApart]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_search.cpp:17]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[DistanceTest.MatchesHandCalculation]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=DistanceTest.MatchesHandCalculation]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[DistanceTest.MatchesHandCalculation]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_search.cpp:23]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[BruteForceTest.EmptyCollectionReturnsNothing]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=BruteForceTest.EmptyCollectionReturnsNothing]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BruteForceTest.EmptyCollectionReturnsNothing]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_search.cpp:29]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[BruteForceTest.SelfMatchComesBackFirstAtZero]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=BruteForceTest.SelfMatchComesBackFirstAtZero]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BruteForceTest.SelfMatchComesBackFirstAtZero]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_search.cpp:34]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[BruteForceTest.ResultsAreSortedNearestFirst]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=BruteForceTest.ResultsAreSortedNearestFirst]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BruteForceTest.ResultsAreSortedNearestFirst]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_search.cpp:47]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[BruteForceTest.RespectsK]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=BruteForceTest.RespectsK]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BruteForceTest.RespectsK]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_search.cpp:65]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[BruteForceTest.MismatchedDimensionsAreSkipped]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=BruteForceTest.MismatchedDimensionsAreSkipped]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[BruteForceTest.MismatchedDimensionsAreSkipped]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_search.cpp:75]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[HnswTest.EmptyIndexReturnsNothing]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=HnswTest.EmptyIndexReturnsNothing]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[HnswTest.EmptyIndexReturnsNothing]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_hnsw.cpp:29]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[HnswTest.SingleVectorIsFound]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=HnswTest.SingleVectorIsFound]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[HnswTest.SingleVectorIsFound]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_hnsw.cpp:35]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[HnswTest.SelfMatchAlwaysSucceeds]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=HnswTest.SelfMatchAlwaysSucceeds]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[HnswTest.SelfMatchAlwaysSucceeds]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_hnsw.cpp:50]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[HnswTest.NoIsolatedNodesAtLayerZero]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=HnswTest.NoIsolatedNodesAtLayerZero]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[HnswTest.NoIsolatedNodesAtLayerZero]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_hnsw.cpp:66]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[HnswTest.GraphIsFullyReachableAtLayerZero]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=HnswTest.GraphIsFullyReachableAtLayerZero]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[HnswTest.GraphIsFullyReachableAtLayerZero]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_hnsw.cpp:82]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[HnswTest.RecallAgainstBruteForceIsHigh]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=HnswTest.RecallAgainstBruteForceIsHigh]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[HnswTest.RecallAgainstBruteForceIsHigh]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_hnsw.cpp:106]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[HnswTest.EfIsRaisedToKWhenTooSmall]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=HnswTest.EfIsRaisedToKWhenTooSmall]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[HnswTest.EfIsRaisedToKWhenTooSmall]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_hnsw.cpp:141]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[QuantizerTest.EncodeBeforeTrainThrows]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=QuantizerTest.EncodeBeforeTrainThrows]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[QuantizerTest.EncodeBeforeTrainThrows]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_quantizer.cpp:27]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[QuantizerTest.TrainOnEmptySampleThrows]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=QuantizerTest.TrainOnEmptySampleThrows]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[QuantizerTest.TrainOnEmptySampleThrows]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_quantizer.cpp:32]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[QuantizerTest.OneByteBerDimension]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=QuantizerTest.OneByteBerDimension]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[QuantizerTest.OneByteBerDimension]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_quantizer.cpp:37]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[QuantizerTest.ErrorStaysWithinTheoreticalBound]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=QuantizerTest.ErrorStaysWithinTheoreticalBound]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[QuantizerTest.ErrorStaysWithinTheoreticalBound]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_quantizer.cpp:48]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[QuantizerTest.ValuesOutsideTrainedRangeAreClamped]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=QuantizerTest.ValuesOutsideTrainedRangeAreClamped]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[QuantizerTest.ValuesOutsideTrainedRangeAreClamped]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_quantizer.cpp:64]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[QuantizerTest.DistanceToQueryMatchesDecodeThenCompute]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=QuantizerTest.DistanceToQueryMatchesDecodeThenCompute]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[QuantizerTest.DistanceToQueryMatchesDecodeThenCompute]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_quantizer.cpp:77]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[QuantizerTest.ZeroRangeDimensionDoesNotDivideByZero]=]  /Users/amankarki/Downloads/projects/lattice/build-test/core/tests/lattice_tests [==[--gtest_filter=QuantizerTest.ZeroRangeDimensionDoesNotDivideByZero]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[QuantizerTest.ZeroRangeDimensionDoesNotDivideByZero]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/amankarki/Downloads/projects/lattice/core/tests/test_quantizer.cpp:98]==]
    WORKING_DIRECTORY [==[/Users/amankarki/Downloads/projects/lattice/build-test/core/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
set(lattice_tests_TESTS [==[WalTest.ReplayOnEmptyFileReturnsNothing]==] [==[WalTest.SingleRecordRoundTrips]==] [==[WalTest.ManyRecordsReplayInOrder]==] [==[WalTest.TornFinalRecordIsDroppedButEarlierOnesSurvive]==] [==[WalTest.AppendsRatherThanOverwrites]==] [==[SegmentTest.MissingFileIsNotAnError]==] [==[SegmentTest.RoundTripsAllRecords]==] [==[SegmentTest.CorruptMagicIsRejected]==] [==[SegmentTest.EmptySegmentIsValid]==] [==[DistanceTest.IdenticalVectorsAreZeroApart]==] [==[DistanceTest.MatchesHandCalculation]==] [==[BruteForceTest.EmptyCollectionReturnsNothing]==] [==[BruteForceTest.SelfMatchComesBackFirstAtZero]==] [==[BruteForceTest.ResultsAreSortedNearestFirst]==] [==[BruteForceTest.RespectsK]==] [==[BruteForceTest.MismatchedDimensionsAreSkipped]==] [==[HnswTest.EmptyIndexReturnsNothing]==] [==[HnswTest.SingleVectorIsFound]==] [==[HnswTest.SelfMatchAlwaysSucceeds]==] [==[HnswTest.NoIsolatedNodesAtLayerZero]==] [==[HnswTest.GraphIsFullyReachableAtLayerZero]==] [==[HnswTest.RecallAgainstBruteForceIsHigh]==] [==[HnswTest.EfIsRaisedToKWhenTooSmall]==] [==[QuantizerTest.EncodeBeforeTrainThrows]==] [==[QuantizerTest.TrainOnEmptySampleThrows]==] [==[QuantizerTest.OneByteBerDimension]==] [==[QuantizerTest.ErrorStaysWithinTheoreticalBound]==] [==[QuantizerTest.ValuesOutsideTrainedRangeAreClamped]==] [==[QuantizerTest.DistanceToQueryMatchesDecodeThenCompute]==] [==[QuantizerTest.ZeroRangeDimensionDoesNotDivideByZero]==])
