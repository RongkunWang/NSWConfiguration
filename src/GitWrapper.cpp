#include "NSWConfiguration/GitWrapper.h"

#include <filesystem>

#include <fmt/core.h>

namespace fs = std::filesystem;

nsw::git::GitInterface::GitInterface() : m_root{nullptr, 0, 0}
{
  git_libgit2_init();
}

nsw::git::GitInterface::~GitInterface()
{
  git_buf_free(&m_root);
  git_libgit2_shutdown();
}

std::string nsw::git::GitInterface::get_git_revision(const std::string& file)
{
  const auto reporoot = check_repository(file);

  auto repo{GitRepoWrapper(reporoot)};

  auto walker{RevWalkerWrapper(repo.get())};
  walker.push_head();

  git_oid oid;
  walker.walk_next(&oid);

  constexpr static auto GITHASHLENGTH = 12;
  std::array<char, GITHASHLENGTH + 1> shortsha{0};
  git_oid_tostr(shortsha.data(), GITHASHLENGTH, &oid);

  auto gitsha = fmt::format("{}", shortsha.data());

  auto object{GitObjectWrapper(repo.get())};
  auto* tree = object.repo_tree();

  const auto cfg_file_name = [&file, &reporoot]() -> std::string {
    const auto GIT_REPO = std::string{".git/"};
    const auto path = fs::path{file};
    const auto abspath = std::string{fs::canonical(path)};
    const auto repo_base = reporoot.substr(0, reporoot.size() - GIT_REPO.size());
    if (abspath.rfind(repo_base) != std::string::npos) {
      return abspath.substr(repo_base.size());
    } else {
      return abspath;
    }
  }();

  auto entry{TreeEntryWrapper()};
  const auto is_tracked = entry.is_tracked(tree, cfg_file_name);

  if (is_tracked) {
    return fmt::format("{} has git SHA: {}", file, gitsha);
  } else {
    return fmt::format("{} (untracked) in repository with git SHA: {}", file, gitsha);
  }
}

std::string nsw::git::GitInterface::check_repository(const std::string& file)
{
  const auto check = git_repository_discover(&m_root, file.c_str(), 0, nullptr);
  if (check == 0) {
    return std::string(m_root.ptr);
  } else {
    throw nsw::git::exceptions::RepositoryNotFound(
      fmt::format("Unable to find repository for {}", file));
  }
}

// GitRepoWrapper
nsw::git::GitInterface::GitRepoWrapper::~GitRepoWrapper()
{
  if (m_repo != nullptr) {
    git_repository_free(m_repo);
  }
}

[[nodiscard]] git_repository* nsw::git::GitInterface::GitRepoWrapper::get() const
{
  if (not m_opened) {
    throw nsw::git::exceptions::RepositoryNotOpened(
      fmt::format("Unable to open repository at {}", m_reporoot));
  }
  return m_repo;
}

bool nsw::git::GitInterface::GitRepoWrapper::open_repo()
{
  return git_repository_open(&m_repo, m_reporoot.c_str()) == 0;
}

// RevWalkerWrapper
nsw::git::GitInterface::RevWalkerWrapper::~RevWalkerWrapper()
{
  if (m_walker != nullptr) {
    git_revwalk_free(m_walker);
  }
}

git_revwalk* nsw::git::GitInterface::RevWalkerWrapper::get()
{
  if (not m_initialized) {
    throw nsw::git::exceptions::RevWalkerNotInitialized("Unable to initialize revwalker");
  }
  return m_walker;
}

void nsw::git::GitInterface::RevWalkerWrapper::push_head()
{
  const auto check = git_revwalk_push_head(get());
  if (check != 0) {
    throw nsw::git::exceptions::RepositoryHeadNotFound("Unable to find repository HEAD");
  }
}

void nsw::git::GitInterface::RevWalkerWrapper::walk_next(git_oid* oid)
{
  const auto check = git_revwalk_next(oid, get());
  if (check != 0) {
    throw nsw::git::exceptions::NextCommitNotFound("Unable to find next commit");
  }
}

bool nsw::git::GitInterface::RevWalkerWrapper::init_walker(git_repository* repo)
{
  return git_revwalk_new(&m_walker, repo) == 0;
}

// GitObjectWrapper
nsw::git::GitInterface::GitObjectWrapper::~GitObjectWrapper()
{
  if (m_object != nullptr) {
    git_object_free(m_object);
  }
}

git_object* nsw::git::GitInterface::GitObjectWrapper::get()
{
  if (not m_valid) {
    throw nsw::git::exceptions::ObjectNotValid("Untrackable object");
  }
  return m_object;
}

git_tree* nsw::git::GitInterface::GitObjectWrapper::repo_tree()
{
  auto* tree = reinterpret_cast<git_tree*>(get());

  [[maybe_unused]]
  const auto count = git_tree_entrycount(tree);
  const auto* entry = git_tree_entry_byindex(tree, 0);

  [[maybe_unused]]
  const auto* name = git_tree_entry_name(entry);

  return tree;
}

bool nsw::git::GitInterface::GitObjectWrapper::get_head(git_repository* repo)
{
  return git_revparse_single(&m_object, repo, "HEAD^{tree}") == 0;
}

// TreeEntryWrapper
nsw::git::GitInterface::TreeEntryWrapper::~TreeEntryWrapper()
{
  if (m_tree_entry != nullptr) {
    git_tree_entry_free(m_tree_entry);
  }
}

bool nsw::git::GitInterface::TreeEntryWrapper::is_tracked(git_tree* tree, const std::string& file)
{
  return (git_tree_entry_bypath(&m_tree_entry, tree, file.c_str()) == 0);
}
