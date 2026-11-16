<?xml version='1.0' encoding='UTF-8' standalone='yes' ?>
<tagfile doxygen_version="1.12.0">
  <compound kind="file">
    <name>define_non_target_part.hpp</name>
    <path>tatami_test/</path>
    <filename>define__non__target__part_8hpp.html</filename>
    <includes id="utils_8hpp" name="utils.hpp" local="yes" import="no" module="no" objc="no">utils.hpp</includes>
  </compound>
  <compound kind="file">
    <name>fetch.hpp</name>
    <path>tatami_test/</path>
    <filename>fetch_8hpp.html</filename>
    <class kind="struct">tatami_test::SparseVector</class>
  </compound>
  <compound kind="file">
    <name>ForcedOracleWrapper.hpp</name>
    <path>tatami_test/</path>
    <filename>ForcedOracleWrapper_8hpp.html</filename>
    <includes id="utils_8hpp" name="utils.hpp" local="yes" import="no" module="no" objc="no">utils.hpp</includes>
    <class kind="class">tatami_test::ForcedOracleWrapper</class>
  </compound>
  <compound kind="file">
    <name>ReversedIndicesWrapper.hpp</name>
    <path>tatami_test/</path>
    <filename>ReversedIndicesWrapper_8hpp.html</filename>
    <includes id="utils_8hpp" name="utils.hpp" local="yes" import="no" module="no" objc="no">utils.hpp</includes>
    <class kind="class">tatami_test::ReversedIndicesWrapper</class>
  </compound>
  <compound kind="file">
    <name>simulate_compressed_sparse.hpp</name>
    <path>tatami_test/</path>
    <filename>simulate__compressed__sparse_8hpp.html</filename>
    <includes id="utils_8hpp" name="utils.hpp" local="yes" import="no" module="no" objc="no">utils.hpp</includes>
    <class kind="struct">tatami_test::SimulateCompressedSparseOptions</class>
    <class kind="struct">tatami_test::SimulateCompressedSparseResult</class>
  </compound>
  <compound kind="file">
    <name>simulate_vector.hpp</name>
    <path>tatami_test/</path>
    <filename>simulate__vector_8hpp.html</filename>
    <includes id="utils_8hpp" name="utils.hpp" local="yes" import="no" module="no" objc="no">utils.hpp</includes>
    <class kind="struct">tatami_test::SimulateVectorOptions</class>
  </compound>
  <compound kind="file">
    <name>test_access.hpp</name>
    <path>tatami_test/</path>
    <filename>test__access_8hpp.html</filename>
    <includes id="define__non__target__part_8hpp" name="define_non_target_part.hpp" local="yes" import="no" module="no" objc="no">define_non_target_part.hpp</includes>
    <includes id="utils_8hpp" name="utils.hpp" local="yes" import="no" module="no" objc="no">utils.hpp</includes>
    <class kind="struct">tatami_test::TestAccessOptions</class>
  </compound>
  <compound kind="file">
    <name>test_unsorted_access.hpp</name>
    <path>tatami_test/</path>
    <filename>test__unsorted__access_8hpp.html</filename>
    <includes id="define__non__target__part_8hpp" name="define_non_target_part.hpp" local="yes" import="no" module="no" objc="no">define_non_target_part.hpp</includes>
    <includes id="test__access_8hpp" name="test_access.hpp" local="yes" import="no" module="no" objc="no">test_access.hpp</includes>
  </compound>
  <compound kind="file">
    <name>throws_error.hpp</name>
    <path>tatami_test/</path>
    <filename>throws__error_8hpp.html</filename>
  </compound>
  <compound kind="file">
    <name>utils.hpp</name>
    <path>tatami_test/</path>
    <filename>utils_8hpp.html</filename>
  </compound>
  <compound kind="class">
    <name>tatami_test::ForcedOracleWrapper</name>
    <filename>classtatami__test_1_1ForcedOracleWrapper.html</filename>
    <templarg>typename Value_</templarg>
    <templarg>typename Index_</templarg>
    <base>tatami::Matrix&lt; Value_, Index_ &gt;</base>
    <member kind="function">
      <type></type>
      <name>ForcedOracleWrapper</name>
      <anchorfile>classtatami__test_1_1ForcedOracleWrapper.html</anchorfile>
      <anchor>af94bb7adddc01275a34e57470c1b138c</anchor>
      <arglist>(std::shared_ptr&lt; const tatami::Matrix&lt; Value_, Index_ &gt; &gt; matrix)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>tatami_test::ReversedIndicesWrapper</name>
    <filename>classtatami__test_1_1ReversedIndicesWrapper.html</filename>
    <templarg>typename Value_</templarg>
    <templarg>typename Index_</templarg>
    <base>tatami::Matrix&lt; Value_, Index_ &gt;</base>
    <member kind="function">
      <type></type>
      <name>ReversedIndicesWrapper</name>
      <anchorfile>classtatami__test_1_1ReversedIndicesWrapper.html</anchorfile>
      <anchor>a9c7a3c529eb78382631547da1da564fc</anchor>
      <arglist>(std::shared_ptr&lt; const tatami::Matrix&lt; Value_, Index_ &gt; &gt; matrix)</arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>tatami_test::SimulateCompressedSparseOptions</name>
    <filename>structtatami__test_1_1SimulateCompressedSparseOptions.html</filename>
    <member kind="variable">
      <type>double</type>
      <name>lower</name>
      <anchorfile>structtatami__test_1_1SimulateCompressedSparseOptions.html</anchorfile>
      <anchor>a1ff7d8426662bb8e75150350d457d40a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>upper</name>
      <anchorfile>structtatami__test_1_1SimulateCompressedSparseOptions.html</anchorfile>
      <anchor>a44cde105ef60528c8ae39da53ee359dc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>density</name>
      <anchorfile>structtatami__test_1_1SimulateCompressedSparseOptions.html</anchorfile>
      <anchor>ac9b8c0023b59db929b693332f42ea3e3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SeedType</type>
      <name>seed</name>
      <anchorfile>structtatami__test_1_1SimulateCompressedSparseOptions.html</anchorfile>
      <anchor>a7aae8258e29cea76e6a1cdb2d49edec1</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>tatami_test::SimulateCompressedSparseResult</name>
    <filename>structtatami__test_1_1SimulateCompressedSparseResult.html</filename>
    <templarg>typename Value_</templarg>
    <templarg>typename Index_</templarg>
    <member kind="variable">
      <type>std::vector&lt; Value_ &gt;</type>
      <name>data</name>
      <anchorfile>structtatami__test_1_1SimulateCompressedSparseResult.html</anchorfile>
      <anchor>ad09e8f082808587815798374dfad8338</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>std::vector&lt; Index_ &gt;</type>
      <name>index</name>
      <anchorfile>structtatami__test_1_1SimulateCompressedSparseResult.html</anchorfile>
      <anchor>aef902079cd5849e69251639f61644468</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>std::vector&lt; std::size_t &gt;</type>
      <name>indptr</name>
      <anchorfile>structtatami__test_1_1SimulateCompressedSparseResult.html</anchorfile>
      <anchor>a958f6659005379da6e5aa0a5db91181c</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>tatami_test::SimulateVectorOptions</name>
    <filename>structtatami__test_1_1SimulateVectorOptions.html</filename>
    <member kind="variable">
      <type>double</type>
      <name>lower</name>
      <anchorfile>structtatami__test_1_1SimulateVectorOptions.html</anchorfile>
      <anchor>a4bc0678d456ffab766ca4e684c300d76</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>upper</name>
      <anchorfile>structtatami__test_1_1SimulateVectorOptions.html</anchorfile>
      <anchor>a297d554e6333c808bdfd14b32175972e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>double</type>
      <name>density</name>
      <anchorfile>structtatami__test_1_1SimulateVectorOptions.html</anchorfile>
      <anchor>a4af160216796b467c9f0e8d4d62b56f1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SeedType</type>
      <name>seed</name>
      <anchorfile>structtatami__test_1_1SimulateVectorOptions.html</anchorfile>
      <anchor>a83ac7134dd316308c639aad7e35c5d22</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>tatami_test::SparseVector</name>
    <filename>structtatami__test_1_1SparseVector.html</filename>
    <templarg>typename Value_</templarg>
    <templarg>typename Index_</templarg>
    <member kind="variable">
      <type>std::vector&lt; Value_ &gt;</type>
      <name>value</name>
      <anchorfile>structtatami__test_1_1SparseVector.html</anchorfile>
      <anchor>ad7a9e16b2259eba35ee81ae9d8ec4ac7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>std::vector&lt; Index_ &gt;</type>
      <name>index</name>
      <anchorfile>structtatami__test_1_1SparseVector.html</anchorfile>
      <anchor>a8c1e0055e8e661685dc4456ef675bd64</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>tatami_test::TestAccessOptions</name>
    <filename>structtatami__test_1_1TestAccessOptions.html</filename>
    <member kind="variable">
      <type>bool</type>
      <name>use_oracle</name>
      <anchorfile>structtatami__test_1_1TestAccessOptions.html</anchorfile>
      <anchor>a5819b7ec7d7ae7cc841cb0b3a2512dcd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>TestAccessOrder</type>
      <name>order</name>
      <anchorfile>structtatami__test_1_1TestAccessOptions.html</anchorfile>
      <anchor>aeefefe05e8501ffe969d166fc48bfb03</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>jump</name>
      <anchorfile>structtatami__test_1_1TestAccessOptions.html</anchorfile>
      <anchor>a62fd941d5a2bce546f534b409073eaba</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bool</type>
      <name>check_sparse</name>
      <anchorfile>structtatami__test_1_1TestAccessOptions.html</anchorfile>
      <anchor>ada12c5cca227834bd6e3c1a8c456bc35</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>std::optional&lt; SeedType &gt;</type>
      <name>seed</name>
      <anchorfile>structtatami__test_1_1TestAccessOptions.html</anchorfile>
      <anchor>a0495964403236468e0951ab5c8a47a82</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="page">
    <name>index</name>
    <title>Testing framework for tatami</title>
    <filename>index.html</filename>
    <docanchor file="index.html">md__2github_2workspace_2README</docanchor>
  </compound>
</tagfile>
