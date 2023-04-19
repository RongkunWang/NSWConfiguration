#ifndef NSWCONFIGURATION_GITWRAPPER_H
#define NSWCONFIGURATION_GITWRAPPER_H

#include <stdexcept>
#include <string>

#include <git2.h>

namespace nsw::git {
  namespace exceptions {
    class LibGit2WrapperException : public std::runtime_error
    {
    public:
      explicit LibGit2WrapperException(std::string msg) : std::runtime_error(std::move(msg)){};
    };
    class RepositoryNotFound : public LibGit2WrapperException
    {
    public:
      explicit RepositoryNotFound(std::string msg) : LibGit2WrapperException(std::move(msg)){};
    };
    class RepositoryNotOpened : public LibGit2WrapperException
    {
    public:
      explicit RepositoryNotOpened(std::string msg) : LibGit2WrapperException(std::move(msg)){};
    };
    class RevWalkerNotInitialized : public LibGit2WrapperException
    {
    public:
      explicit RevWalkerNotInitialized(std::string msg) : LibGit2WrapperException(std::move(msg)){};
    };
    class RepositoryHeadNotFound : public LibGit2WrapperException
    {
    public:
      explicit RepositoryHeadNotFound(std::string msg) : LibGit2WrapperException(std::move(msg)){};
    };
    class NextCommitNotFound : public LibGit2WrapperException
    {
    public:
      explicit NextCommitNotFound(std::string msg) : LibGit2WrapperException(std::move(msg)){};
    };
    class ObjectNotValid : public LibGit2WrapperException
    {
    public:
      explicit ObjectNotValid(std::string msg) : LibGit2WrapperException(std::move(msg)){};
    };
  }  // namespace exceptions

  /*!
   * \brief Class to provide an interface to libgit2
   */
  class GitInterface
  {
  public:
    GitInterface();
    ~GitInterface();

    /*!
     * \brief Extract the git SHA for a file
     *
     * \param file path to a file to extract the git SHA of.
     *
     * \returns a string containing the git information about the file.
     *          - If the file belongs to the repository, it will be reported.
     *            <file> has git SHA: commit SHA
     *          - If the file exists in a directory that is a git repository, but
     *            the file is not in the repository tree, it will note this.
     *            <file> (untracked) in repository git SHA: commit SHA
     *          - If no git information can be extracted, this will be reported.
     *            Unable to find repository for <file>

     */
    std::string get_git_revision(const std::string& file);

  private:
    /*!
     * \brief Find the repository root for a given file, used
     *        internally when extracting the git revision.
     *
     * \param file path to a file to determine the repository root of
     *
     * \throws nsw::git::exceptions::RepositoryNotFound if a git
     *         repository can't be found in the vicinity of file
     */
    std::string check_repository(const std::string& file);

    git_buf m_root;

  private:
    /*!
     * \brief RAII wrapper around the libgit2::git_repository object to
     *        ensure that resources are properly deallocated in
     *        case of failure
     */
    class GitRepoWrapper
    {

    public:
      explicit GitRepoWrapper(std::string reporoot) :
        m_reporoot{std::move(reporoot)}, m_opened{open_repo()} {};

      ~GitRepoWrapper();

      GitRepoWrapper(const GitRepoWrapper&) = delete;
      GitRepoWrapper(GitRepoWrapper&&) = delete;
      GitRepoWrapper& operator=(const GitRepoWrapper&) = delete;
      GitRepoWrapper& operator=(GitRepoWrapper&&) = delete;

      /*!
       * \brief Get the git_repository pointer if it properly opened
       *
       * \returns the raw pointer to the wrapped libgit2::git_repository object
       */
      [[nodiscard]] git_repository* get() const;

    private:
      /*!
       * \brief Calls git_repository_open and returns true if successful
       */
      bool open_repo();

      std::string m_reporoot{};
      git_repository* m_repo{nullptr};
      bool m_opened{};
    };

    /*!
     * \brief RAII wrapper around the libgit2::git_revwalk object to
     *        ensure that resources are properly deallocated in
     *        case of failure
     */
    class RevWalkerWrapper
    {

    public:
      explicit RevWalkerWrapper(git_repository* repo) : m_initialized{init_walker(repo)} {};

      ~RevWalkerWrapper();

      RevWalkerWrapper(const RevWalkerWrapper&) = delete;
      RevWalkerWrapper(RevWalkerWrapper&&) = delete;
      RevWalkerWrapper& operator=(const RevWalkerWrapper&) = delete;
      RevWalkerWrapper& operator=(RevWalkerWrapper&&) = delete;

      /*!
       * \brief Get the git_revwalk pointer if it properly initialized
       *
       * \returns the raw pointer to the wrapped libgit2::git_revwalk object
       *
       * \throws RevWalkerNotInitialized in case of failure
       */
      [[nodiscard]] git_revwalk* get();

      /*!
       * \brief Calls git_revwalk_push_head to try to move to the HEAD
       *
       * \throws nsw::git::exceptions::RepositoryHeadNotFound in case of failure
       */
      void push_head();

      /*!
       * \brief Calls git_revwalk_next to try to move to the next commit
       *
       * \throw NextCommitNotFound in case of failure
       */
      void walk_next(git_oid* oid);

    private:
      /*!
       * \brief Calls git_revwalk_new to try to initialize the revwalker
       *
       * \returns true if the revwalker initializes
       */
      bool init_walker(git_repository* repo);

      git_revwalk* m_walker{nullptr};
      bool m_initialized{};
    };


    /*!
     * \brief RAII wrapper around the libgit2::git_object object to
     *        ensure that resources are properly deallocated in
     *        case of failure
     */
    class GitObjectWrapper
    {

    public:
      explicit GitObjectWrapper(git_repository* repo) : m_valid{get_head(repo)} {};

      ~GitObjectWrapper();

      GitObjectWrapper(const GitObjectWrapper&) = delete;
      GitObjectWrapper(GitObjectWrapper&&) = delete;
      GitObjectWrapper& operator=(const GitObjectWrapper&) = delete;
      GitObjectWrapper& operator=(GitObjectWrapper&&) = delete;

      /*!
       * \brief Get the git_object pointer if it is valid
       *
       * \returns the raw pointer to the wrapped libgit2::git_object object
       *
       * \throws ObjectNotValid in case of failure
       */
      [[nodiscard]] git_object* get();

      /*!
       * \brief Get the git_tree pointer corresponding to this git_object
       *
       */
      git_tree* repo_tree();

    private:
      /*!
       * \brief Calls libgit2::git_revparse_single to get the object at the HEAD
       *
       * \throws ObjectNotValid in case of failure
       */
      bool get_head(git_repository* repo);

      git_object* m_object{nullptr};
      bool m_valid{};
    };


    /*!
     * \brief RAII wrapper around the libgit2::git_tree_entry object to
     *        ensure that resources are properly deallocated in
     *        case of failure
     */
    class TreeEntryWrapper
    {

    public:
      TreeEntryWrapper() = default;

      ~TreeEntryWrapper();

      TreeEntryWrapper(const TreeEntryWrapper&) = delete;
      TreeEntryWrapper(TreeEntryWrapper&&) = delete;
      TreeEntryWrapper& operator=(const TreeEntryWrapper&) = delete;
      TreeEntryWrapper& operator=(TreeEntryWrapper&&) = delete;

      /*!
       * \brief Get the git_tree_entry pointer if it corresponds to a tracked object
       *
       * \returns the raw pointer to the wrapped libgit2::git_tree_entry object
       */
      [[nodiscard]] git_tree_entry* get() { return m_tree_entry; };

      /*!
       * \brief Calls git_tree_entry_bypath to determine if a file is tracked in the git tree
       *
       * \returns true in case of success
       */
      bool is_tracked(git_tree* tree, const std::string& file);

    private:
      git_tree_entry* m_tree_entry{nullptr};
    };
  };
}  // namespace nsw::git

#endif
