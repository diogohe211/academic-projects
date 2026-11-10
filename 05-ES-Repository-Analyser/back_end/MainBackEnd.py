from pydriller import *
import os
from complexipy import code_complexity
from git import InvalidGitRepositoryError

class MainBackEnd:
    """
    This class is responsible for getting all the information from the local repository.

    It uses different libraries to give the needed results.

    Attributes:
        repo_path (str): The path to the repository.
        all_files (dict): A dictionary of all the files in the repository and their characteristics.
        current_commit : hash of the were all files were updated.
    """

    def __init__(self, repo_path=None):
        
        if repo_path is None:
            repo_path = self.find_git_root()
        self.repo_path = repo_path
        self.all_files = {}
        self.all_files_history = {}
        self.current_commit = None

    def find_git_root(self):
        current_path = os.path.abspath(os.getcwd())
        while current_path != os.path.dirname(current_path):
            if os.path.isdir(os.path.join(current_path, ".git")):
                return current_path
            current_path = os.path.dirname(current_path)
        raise InvalidGitRepositoryError("No git repository found")

    def get_all_files(self):
        """
        This method returns all files from the repository while verifying if the variable is correctly updated.
        :return: nothing since all the changes will be stored in the all_files attribute.
        """
        # Initializes the variable if it has not started or updates it
        if self.current_commit is None:
            self.init_all_files()
        else:
            self.update()
        return self.all_files

    def get_all_files_history(self):
        """
        This method returns all the files history from the repository while verifying if the variable is correctly updated.
        :return: nothing since all the changes will be stored in the all_files_history attribute.
        """
        # Initializes the variable if it has not started or upadtes it
        if self.current_commit is None:
            self.init_all_files()
        else:
            self.update()
        return self.all_files_history

    def init_all_files(self):
        """
        This method initializes all files from the repository, it shall only be used at the start since it is takes some time to load.
        :return: nothing since all the changes will be stored in the all_files attribute.
        """
        # Traverse the repository
        final_commit = self.current_commit
        for commit in Repository(self.repo_path).traverse_commits():
            # if not has_changed_cur_commit:
            #     self.current_commit = commit.hash
            #     print(commit.author_date.date().isoformat())
            #     has_changed_cur_commit = True
            for modification in commit.modified_files:
                self.modify_all_files(modification, commit)
                self.modify_all_files_complexity(modification, commit)
            final_commit = commit
        self.current_commit = final_commit.hash


    def update(self):
        """
        Updates the files with the last commit
        :return: nothing since all the changes will be stored in the all_files attribute.
        """
        temp_repository = Repository(self.repo_path, from_commit=self.current_commit)


        #Saving last commit
        temp_commit = None
        #Iterating through commits until it reaches the last commit
        for commit in temp_repository.traverse_commits():# update last commit changes
            for modification in commit.modified_files:
                self.modify_all_files(modification, commit)
                self.modify_all_files_complexity(modification, commit)
            temp_commit = commit
        #Upadting the commit attribute
        self.current_commit = temp_commit.hash

    def modify_all_files(self, modification, commit):
        """
        This method updates the files from a specific commit modification, it will act depending on the type of modification.
        :param modification: modification object to be updated.
        :param commit: commit object to be updated.
        :return: nothing since all the changes will be stored in the all_files attribute.
        """
        change_type = modification.change_type.name
        if change_type == "ADD":
            new_file = dict(filename=modification.filename,
                            filesize=self.get_file_size(modification.source_code),
                            nloc=modification.nloc,
                            cycComp=modification.complexity,
                            cogComp=self.get_code_complexity(modification.source_code),
                            author=commit.author.name)
            self.all_files.update({modification.new_path: new_file})
        if change_type == "DELETE" and modification.new_path in self.all_files:
            self.all_files.pop(modification.new_path)
        if change_type == "MODIFY":
            new_file = dict(filename=modification.filename,
                            filesize=self.get_file_size(modification.source_code),
                            nloc=modification.nloc,
                            cycComp=modification.complexity,
                            cogComp=self.get_code_complexity(modification.source_code),
                            author=commit.author.name)
            self.all_files.update({modification.new_path: new_file})
        if change_type == "RENAME" and modification.old_path in self.all_files:
            file_meta_data = self.all_files[modification.old_path]
            self.all_files.pop(modification.old_path)
            self.all_files.update({modification.new_path: file_meta_data})

    def modify_all_files_complexity(self, modification, commit):
        change_type = modification.change_type.name
        if change_type == "ADD":
            new_file = dict(cycCompHist=[modification.complexity],
                            cogCompHist=[self.get_code_complexity(modification.source_code)],
                            nlocHist=[modification.nloc],
                            fileSizeHist=[self.get_file_size(modification.source_code)],
                            commitDate=[commit.author_date.date().isoformat()],)
            self.all_files_history.update({modification.new_path: new_file})
        if change_type == "DELETE" and modification.new_path in self.all_files_history:
            self.all_files_history.pop(modification.new_path)
        if change_type == "MODIFY" and modification.old_path in self.all_files_history:
            self.all_files_history[modification.old_path]["cogCompHist"].append(self.get_code_complexity(modification.source_code))
            self.all_files_history[modification.new_path]["cycCompHist"].append(modification.complexity)
            self.all_files_history[modification.new_path]["nlocHist"].append(modification.nloc)
            self.all_files_history[modification.new_path]["fileSizeHist"].append(self.get_file_size(modification.source_code))
            self.all_files_history[modification.new_path]["commitDate"].append(commit.author_date.date().isoformat())
        if change_type == "RENAME" and modification.old_path in self.all_files_history:
            file_meta_data = self.all_files_history[modification.old_path]
            self.all_files_history.pop(modification.old_path)
            self.all_files_history.update({modification.new_path: file_meta_data})
    # internal methods

    def get_file_size(self, string):
        """
        This method is used internally to calculate the file size of a string.
        :param string: the string to calculate the file size of.
        :return: the file size of the string.
        """
        if string is None:
            return 0
        else:
            return len(string.encode("utf-8"))

    def get_code_complexity(self, string):
        """
        This method is used internally to calculate the code cognitive complexity of a string.
        :param string: the code (String) to calculate the code complexity of.
        :return: the code complexity of the string (returns 'N/A' in case of a non-calculable string).
        """
        if string is None:
            return 0
        else:
            try:
                comp = code_complexity(string).complexity
                if comp is None:
                    return 'N/A'
                else:
                    return comp
            except OSError:
                return "N/A"
            except ValueError:
                return "N/A"




# Method only used for testing
if __name__ == "__main__":
    # Path to your local git repository
    repo_path_final = os.path.abspath("./../../..")

    # Ensure the repository path exists
    if not os.path.exists(repo_path_final):
        print(f"Error: Repository path does not exist: {repo_path_final}")
    else:
        print("Analyzing repository...")
        mainBackEnd = MainBackEnd(repo_path = repo_path_final)
        mainBackEnd.get_all_files()
        print("\nFiles in the repository:")
        for x, obj in mainBackEnd.all_files.items():
            print(x)

            for y in obj:
                print("\t" + y + ':', obj[y])
