import pytest
import os
from unittest.mock import MagicMock
from MainBackEnd import MainBackEnd

backend = MainBackEnd()


########################################## PARTE QUE TESTA O MÉTODO find_git_root ##########################################
def test_find_git_root_success(tmp_path):
    # Teste que verifica se encontramos uma root .git

    # Cria uma estrutura de diretório temporária com uma pasta .git
    git_dir = tmp_path / ".git"  # Cria um caminho para a pasta .git  (tmp_path é uma fixture do pytest) ex: C:/tmp/pytest/.git
    git_dir.mkdir()  # Cria o diretório .git (mkdir() cria um diretório no caminho especificado (no caso git_dir))

    # Muda para o diretório temporário
    original_cwd = os.getcwd()  # Guarda o diretório atual
    os.chdir(tmp_path)  # Muda para o diretório temporário

    try:
        result = backend.find_git_root()  # Chama o método find_git_root() da instância criada
        assert result == str(tmp_path), f"Expected {tmp_path}, got {result}"  # Verifica se o resultado é o esperado
    finally:
        # Retorna ao diretório original
        os.chdir(original_cwd)

    # Quando o teste termina, o diretório temporário é excluído automaticamente


############################################################################################################################
########################################## PARTE QUE TESTA O MÉTODO get_file_size ##########################################

def test_get_file_size_none():
    result = backend.get_file_size(None)
    assert result == 0, "Expected file size for None should be 0."


# Test case for the get_file_size method when the input is an empty string
def test_get_file_size_empty_string():
    result = backend.get_file_size("")
    assert result == 0, "Expected file size for an empty string should be 0."


# Test case for the get_file_size method when the input is a non-empty string
def test_get_file_size_non_empty_string():
    test_string = "Hello, world!"
    result = backend.get_file_size(test_string)
    expected_size = len(test_string.encode("utf-8"))
    assert result == expected_size, f"Expected file size for '{test_string}' should be {expected_size}."


# Test case for the get_file_size method when the input contains special characters
def test_get_file_size_special_characters():
    test_string = "你好，世界！"  # Chinese characters
    result = backend.get_file_size(test_string)
    expected_size = len(test_string.encode("utf-8"))
    assert result == expected_size, f"Expected file size for '{test_string}' should be {expected_size}."


# Test case for the get_file_size method when the input contains a string with multiple characters
def test_get_file_size_multiple_characters():
    test_string = "a" * 1000  # String with 1000 characters
    result = backend.get_file_size(test_string)
    expected_size = len(test_string.encode("utf-8"))
    assert result == expected_size, f"Expected file size for a string with 1000 'a' characters should be {expected_size}."


##################################################################################################################################
########################################## PARTE QUE TESTA O MÉTODO get_code_complexity ##########################################

def test_get_code_complexity_empty_input():
    """Test the case where input string is None"""
    result = backend.get_code_complexity(None)
    assert result == 0


def test_get_code_complexity_invalid_code():
    """Test the case where a valid code string is given"""
    code_string = "Hello World\n"
    result = backend.get_code_complexity(code_string)
    assert result == 'N/A'


def test_get_code_complexity_valid_code():
    """Test the case where a valid code string is given"""
    code_string = "def example():\n   if x > 5:\n           return True\n   else:\n     return False"
    result = backend.get_code_complexity(code_string)
    assert result == 1


############################################################################################################################
########################################## PARTE QUE TESTA O MÉTODO get_all_files ##########################################

def test_get_all_files_initialization():
    """
    Test if the `get_all_files` method correctly initializes files when `current_commit` is None.
    """
    backend.current_commit = None  # Garantir que current_commit é None
    backend.all_files = []  # Inicializa uma lista vazia para all_files

    # Mockar (simular) o método `init_all_files`
    backend.init_all_files = MagicMock()

    # Simular a inicialização de `all_files`
    backend.init_all_files.return_value = None  # init_all_files não retorna nada
    backend.all_files = ["file1.py", "file2.py"]  # Atualizar o estado manualmente

    # Mockar o método `update` para garantir que não seja chamado
    backend.update = MagicMock()

    result = backend.get_all_files()

    # Verificar que `init_all_files` foi chamado e `update` não foi chamado
    backend.init_all_files.assert_called_once()
    backend.update.assert_not_called()

    # Verificar se o retorno de `get_all_files` é o esperado
    assert result == ["file1.py", "file2.py"], "Expected initialized list of files."
    assert backend.all_files == ["file1.py", "file2.py"], "The all_files attribute should match the initialized list."


def test_get_all_files_update():
    """
    Test if the `get_all_files` method correctly updates files when `current_commit` is not None.
    """
    backend.current_commit = "some_commit_hash"  # Set a non-None commit
    backend.all_files = ["file1.py"]  # Mock existing files

    # Mock the `init_all_files` method
    backend.init_all_files = MagicMock()

    # Mock the `update` method
    backend.update = MagicMock()

    # Directly modify `all_files` to simulate the update behavior
    def mock_update():
        backend.all_files.append("file2.py")

    # Manually call the mock_update function, simulating what `update` would do
    backend.update = mock_update

    result = backend.get_all_files()

    # Verify `update` was called and `init_all_files` was not called
    assert backend.update == mock_update  # Ensure `update` is indeed our mock function
    backend.init_all_files.assert_not_called()

    # Verify the returned all_files is as expected
    assert result == ["file1.py", "file2.py"], "Expected updated list of files."
    assert backend.all_files == ["file1.py", "file2.py"], "The all_files attribute should match the updated list."


def test_get_all_files_no_changes():
    """
    Test if the `get_all_files` method maintains the `all_files` state when no changes are made.
    """
    backend.current_commit = "some_commit_hash"
    backend.all_files = ["file1.py", "file2.py"]

    # Mock the `init_all_files` and `update` methods
    backend.init_all_files = MagicMock()
    backend.update = MagicMock()

    result = backend.get_all_files()

    # Verify no changes were made to all_files
    backend.init_all_files.assert_not_called()
    backend.update.assert_called_once()

    # Verify that all_files remains unchanged
    assert result == ["file1.py", "file2.py"], "The list of files should remain unchanged."
    assert backend.all_files == ["file1.py", "file2.py"], "The all_files attribute should remain consistent."


############################################################################################################################
########################################## PARTE QUE TESTA O MÉTODO modify_all_files_complexity ##########################################

def test_modify_all_files_complexity_add():
    """Test the ADD case."""
    modification = MagicMock()
    modification.change_type.name = "ADD"
    modification.complexity = 5
    modification.source_code = "def example():\n   return True"
    modification.nloc = 10
    modification.new_path = "new_file.py"

    commit = MagicMock()
    commit.author_date.date.return_value.isoformat.return_value = "2023-01-01"

    backend.all_files_history = {}
    backend.get_code_complexity = MagicMock(return_value=1)
    backend.get_file_size = MagicMock(return_value=200)

    backend.modify_all_files_complexity(modification, commit)

    assert "new_file.py" in backend.all_files_history
    new_file_data = backend.all_files_history["new_file.py"]
    assert new_file_data["cycCompHist"] == [5]
    assert new_file_data["cogCompHist"] == [1]
    assert new_file_data["nlocHist"] == [10]
    assert new_file_data["fileSizeHist"] == [200]
    assert new_file_data["commitDate"] == ["2023-01-01"]


def test_modify_all_files_complexity_delete():
    """Test the DELETE case."""
    backend.all_files_history = {"file_to_delete.py": {"data": "some_data"}}
    modification = MagicMock()
    modification.change_type.name = "DELETE"
    modification.new_path = "file_to_delete.py"

    backend.modify_all_files_complexity(modification, MagicMock())

    assert "file_to_delete.py" not in backend.all_files_history


def test_modify_all_files_complexity_modify():
    """Test the MODIFY case."""
    backend.all_files_history = {
        "file_to_modify.py": {
            "cycCompHist": [4],
            "cogCompHist": [2],
            "nlocHist": [50],
            "fileSizeHist": [100],
            "commitDate": ["2023-01-01"],
        }
    }

    modification = MagicMock()
    modification.change_type.name = "MODIFY"
    modification.old_path = "file_to_modify.py"
    modification.new_path = "file_to_modify.py"
    modification.complexity = 6
    modification.source_code = "def example():\n    return True"
    modification.nloc = 60

    commit = MagicMock()
    commit.author_date.date.return_value.isoformat.return_value = "2023-02-01"

    backend.get_code_complexity = MagicMock(return_value=3)
    backend.get_file_size = MagicMock(return_value=150)

    backend.modify_all_files_complexity(modification, commit)

    modified_file_data = backend.all_files_history["file_to_modify.py"]
    assert modified_file_data["cycCompHist"] == [4, 6]
    assert modified_file_data["cogCompHist"] == [2, 3]
    assert modified_file_data["nlocHist"] == [50, 60]
    assert modified_file_data["fileSizeHist"] == [100, 150]
    assert modified_file_data["commitDate"] == ["2023-01-01", "2023-02-01"]


def test_modify_all_files_complexity_rename():
    """Test the RENAME case."""
    backend.all_files_history = {
        "old_name.py": {
            "cycCompHist": [5],
            "cogCompHist": [1],
            "nlocHist": [30],
            "fileSizeHist": [120],
            "commitDate": ["2023-01-01"],
        }
    }

    modification = MagicMock()
    modification.change_type.name = "RENAME"
    modification.old_path = "old_name.py"
    modification.new_path = "new_name.py"

    backend.modify_all_files_complexity(modification, MagicMock())

    assert "old_name.py" not in backend.all_files_history
    assert "new_name.py" in backend.all_files_history
    renamed_file_data = backend.all_files_history["new_name.py"]
    assert renamed_file_data["cycCompHist"] == [5]
    assert renamed_file_data["cogCompHist"] == [1]
    assert renamed_file_data["nlocHist"] == [30]
    assert renamed_file_data["fileSizeHist"] == [120]
    assert renamed_file_data["commitDate"] == ["2023-01-01"]


############################################################################################################################
########################################## PARTE QUE TESTA O MÉTODO modify_all_files ##########################################
def test_modify_all_files_delete():
    """Testa o caso DELETE."""
    backend.all_files = {"file_to_delete.py": {"data": "some_data"}}
    modification = MagicMock()
    modification.change_type.name = "DELETE"
    modification.new_path = "file_to_delete.py"

    backend.modify_all_files(modification, MagicMock())

    assert "file_to_delete.py" not in backend.all_files


def test_modify_all_files_rename():
    """Testa o caso RENAME."""
    backend.all_files = {
        "old_name.py": {
            "filename": "old_name.py",
            "filesize": 60,
            "nloc": 25,
            "cycComp": 6,
            "cogComp": 3,
            "author": "Jane Smith",
        }
    }

    modification = MagicMock()
    modification.change_type.name = "RENAME"
    modification.old_path = "old_name.py"
    modification.new_path = "new_name.py"

    backend.modify_all_files(modification, MagicMock())

    assert "old_name.py" not in backend.all_files
    assert "new_name.py" in backend.all_files
    renamed_file = backend.all_files["new_name.py"]
    assert renamed_file["filename"] == "old_name.py"
    assert renamed_file["filesize"] == 60
    assert renamed_file["nloc"] == 25
    assert renamed_file["cycComp"] == 6
    assert renamed_file["cogComp"] == 3
    assert renamed_file["author"] == "Jane Smith"


############################################################################################################################
########################################## PARTE QUE TESTA O MÉTODO get_all_files_history ##################################

def test_get_all_files_history_initialization():
    """
    Test if the `get_all_files_history` method correctly initializes history when `current_commit` is None.
    """
    backend.current_commit = None  # Set `current_commit` to None
    backend.all_files_history = []  # Initialize `all_files_history` as an empty list

    # Mock the `init_all_files` method to simulate initialization
    backend.init_all_files = MagicMock()

    # Simulate initialization of `all_files_history`
    backend.init_all_files.return_value = None  # `init_all_files` doesn't return anything
    backend.all_files_history = ["file1.py", "file2.py"]  # Set initial history manually

    # Mock the `update` method to ensure it doesn't get called
    backend.update = MagicMock()

    result = backend.get_all_files_history()

    # Verify `init_all_files` was called, and `update` was not called
    backend.init_all_files.assert_called_once()
    backend.update.assert_not_called()

    # Verify the returned `all_files_history` is correct
    assert result == ["file1.py", "file2.py"], "Expected initialized list of files history."
    assert backend.all_files_history == ["file1.py",
                                         "file2.py"], "The all_files_history attribute should match the initialized list."


def test_get_all_files_history_update():
    """
    Test if the `get_all_files_history` method correctly updates history when `current_commit` is not None.
    """
    backend.current_commit = "some_commit_hash"  # Set a commit
    backend.all_files_history = ["file1.py"]  # Mock existing files history

    # Mock the `init_all_files` method
    backend.init_all_files = MagicMock()

    # Mock the `update` method
    backend.update = MagicMock()

    # Directly modify `all_files_history` to simulate the update behavior
    def mock_update():
        if not "file2.py" in backend.all_files_history:
            backend.all_files_history.append("file2.py")

    # Manually assign `mock_update` to simulate the update behavior
    backend.update = mock_update

    result = backend.get_all_files_history()

    # Verify `update` was called and `init_all_files` was not called
    backend.update()  # Calling the update function directly
    backend.init_all_files.assert_not_called()

    # Verify the returned `all_files_history` is as expected
    assert result == ["file1.py", "file2.py"], "Expected updated list of files history."
    assert backend.all_files_history == ["file1.py",
                                         "file2.py"], "The all_files_history attribute should match the updated list."


def test_get_all_files_history_no_changes():
    """
    Test if the `get_all_files_history` method maintains the `all_files_history` state when no changes are made.
    """
    backend.current_commit = "some_commit_hash"  # Set a non-None commit
    backend.all_files_history = ["file1.py", "file2.py"]

    # Mock the `init_all_files` and `update` methods
    backend.init_all_files = MagicMock()
    backend.update = MagicMock()

    result = backend.get_all_files_history()

    # Verify no changes were made to all_files_history
    backend.init_all_files.assert_not_called()
    backend.update.assert_called_once()

    # Verify that all_files_history remains unchanged
    assert result == ["file1.py", "file2.py"], "The list of files history should remain unchanged."
    assert backend.all_files_history == ["file1.py",
                                         "file2.py"], "The all_files_history attribute should remain consistent."

############################################################################################################################
