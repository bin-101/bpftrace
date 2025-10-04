// semantic_analyser_harness_commented.cpp
// tests/semantic_analyser.cpp のクラス定義部分をそのまま抜粋し、丁寧にコメントを付けた参考用コードです。
// 実際のビルドやテストには含まれませんが、構造理解の助けになります。

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "arch/arch.h"
#include "ast/ast.h"
#include "ast/attachpoint_parser.h"
#include "ast/passes/c_macro_expansion.h"
#include "ast/passes/clang_parser.h"
#include "ast/passes/field_analyser.h"
#include "ast/passes/fold_literals.h"
#include "ast/passes/import_scripts.h"
#include "ast/passes/map_sugar.h"
#include "ast/passes/named_param.h"
#include "ast/passes/parser.h"
#include "ast/passes/pid_filter_pass.h"
#include "ast/passes/printer.h"
#include "ast/passes/probe_expansion.h"
#include "ast/passes/recursion_check.h"
#include "ast/passes/resolve_imports.h"
#include "ast/passes/resource_analyser.h"
#include "ast/passes/semantic_analyser.h"
#include "ast/passes/type_system.h"
#include "bpftrace.h"
#include "btf_common.h"
#include "driver.h"
#include "mocks.h"

namespace bpftrace::test::semantic_analyser {

using ::testing::_;
using ::testing::HasSubstr;

// テスト用に任意で指定できる追加情報・フラグ。
struct Mock
{
  BPFtrace &bpftrace; // 外部で生成した MockBPFtrace を参照。
};

// unsafe モードを有効にするフラグ（デフォルトは safe）。
enum class UnsafeMode
{
  Enable = 0,
};

// 子プロセス関連のテストを有効にするためのフラグ。
enum class Child
{
  Enable = 0,
};

// BPFfeature を意図的に無効化するときに使う。
enum class NoFeatures
{
  Enable = 0,
};

// 特定の警告が出ることを期待するときに指定。
struct Warning
{
  std::string_view str;
};

// 特定の警告が出ないことを期待するときに指定。
struct NoWarning
{
  std::string_view str;
};

// エラーメッセージを期待するときに指定（空文字なら「何かエラーが発生すること」だけをチェック）。
struct Error
{
  std::string_view str;
};

// 型情報を外部から注入する場合に使用。
struct Types
{
  ast::TypeMetadata &types;
};

// 出力された AST が想定どおりかチェックしたいときに使う。
struct ExpectedAST
{
  std::string_view str;
};

// 可変長引数から特定の型を抽出するユーティリティ（複数指定は不可）。
template <typename T, typename First, typename... Ts>
std::optional<T> extract(First &&arg, Ts &&...rest)
{
  if constexpr (std::is_same_v<std::decay_t<First>, T>) {
    static_assert(!(std::is_same_v<std::decay_t<Ts>, T> || ...),
                  "Only one argument of each type is allowed");
    return arg;
  }
  if constexpr (sizeof...(Ts) != 0) {
    return extract<T, Ts...>(std::forward<Ts>(rest)...);
  }
  return std::nullopt;
}

// 引数が存在しない場合のテンプレート。
template <typename T>
std::optional<T> extract()
{
  return std::nullopt;
}

// 文字列プレフィックスの改行などを整えるユーティリティ。
std::string_view clean_prefix(std::string_view view)
{
  while (!view.empty() && view[0] == '\n')
    view.remove_prefix(1);
  return view;
}

// テスト用のハーネス。入力スクリプトに各種パスを適用して動作を検証する。
class SemanticAnalyserHarness
{
public:
  template <typename... Ts>
    requires((std::is_same_v<std::decay_t<Ts>, Mock> ||
              std::is_same_v<std::decay_t<Ts>, UnsafeMode> ||
              std::is_same_v<std::decay_t<Ts>, Child> ||
              std::is_same_v<std::decay_t<Ts>, NoFeatures> ||
              std::is_same_v<std::decay_t<Ts>, Warning> ||
              std::is_same_v<std::decay_t<Ts>, NoWarning> ||
              std::is_same_v<std::decay_t<Ts>, Error> ||
              std::is_same_v<std::decay_t<Ts>, ExpectedAST> ||
              std::is_same_v<std::decay_t<Ts>, Types>) &&
             ...)
  ast::ASTContext test(std::string_view input, Ts &&...args)
  {
    ast::ASTContext ast("stdin", std::string(clean_prefix(input)));

    bpftrace_.reset();
    types_.reset();

    auto mock = extract<Mock>(args...);
    auto unsafe_mode = extract<UnsafeMode>(args...);
    auto child = extract<Child>(args...);
    auto no_features = extract<NoFeatures>(args...);
    auto warning = extract<Warning>(args...);
    auto nowarning = extract<NoWarning>(args...);
    auto error = extract<Error>(args...);
    auto types = extract<Types>(args...);
    auto expected_ast = extract<ExpectedAST>(args...);

    if (!mock) {
      bpftrace_ = get_mock_bpftrace();
      mock.emplace(*bpftrace_);
    }
    mock->bpftrace.safe_mode_ = !unsafe_mode.has_value();
    mock->bpftrace.feature_ = std::make_unique<MockBPFfeature>(
        !no_features.has_value());
    if (child.has_value()) {
      mock->bpftrace.cmd_ = "not-empty"; // 子プロセス対応のためにセットするフィールド。
    }
    if (!types) {
      types_.emplace();
      types.emplace(*types_);
    }

    auto ok = ast::PassManager()
                  .put(ast)
                  .put(mock->bpftrace)
                  .put(types->types)
                  .add(ast::CreateParsePass()) // ここからパスを順番に適用していく。
                  .add(ast::CreateResolveImportsPass())
                  .add(ast::CreateImportInternalScriptsPass())
                  .add(ast::CreateMacroExpansionPass())
                  .add(ast::CreateParseAttachpointsPass())
                  .add(ast::CreateProbeExpansionPass())
                  .add(ast::CreateFieldAnalyserPass())
                  .add(ast::CreateClangParsePass())
                  .add(ast::CreateCMacroExpansionPass())
                  .add(ast::CreateFoldLiteralsPass())
                  .add(ast::CreateMapSugarPass())
                  .add(ast::CreateNamedParamsPass())
                  .add(ast::CreateSemanticPass()) // 第一回セマンティック解析
                  .add(ast::CreatePidFilterPass())
                  .add(ast::CreateRecursionCheckPass())
                  .add(ast::CreateSemanticPass()) // 再度セマンティック解析を実行
                  .add(ast::CreateResourcePass())
                  .run();

    // エラー・警告の検証は tests/semantic_analyser.cpp 下部で詳細に行われる。
    // ここでは戻り値の AST を返却する。

    std::stringstream out;
    ast.diagnostics().emit(out, ast::Diagnostics::Severity::Error);
    const auto errstr = out.str();
    if (error) {
      if (!error->str.empty()) {
        EXPECT_THAT(errstr, HasSubstr(clean_prefix(error->str))) << errstr;
      } else {
        EXPECT_TRUE(!errstr.empty()) << errstr;
      }
    } else {
      EXPECT_EQ(errstr, "") << errstr;
    }

    return ast;
  }

private:
  std::unique_ptr<MockBPFtrace> bpftrace_;
  std::optional<ast::TypeMetadata> types_;
};

class SemanticAnalyserTest : public SemanticAnalyserHarness,
                             public testing::Test
{};

} // namespace bpftrace::test::semantic_analyser

