import flask
from flask import Flask
from MainBackEnd import MainBackEnd
import json
from flask_cors import CORS


# Having a backEnd object to ensure fast updates
mainBackEnd = MainBackEnd()

# Setting up server
server = Flask(__name__)
CORS(server)

# Setting up a route that will return all the information from all the files
@server.route('/file_names', methods=['POST','GET'])
def file_names():
    # getting all the information needed
    all_files = mainBackEnd.get_all_files()
    return json.dumps({"result":all_files})

@server.route('/file_history', methods=['POST', 'GET'])
def file_history():
    # getting all the file history
    all_files_history = mainBackEnd.get_all_files_history()
    return json.dumps({"result":all_files_history})

@server.route('/file_data_tables', methods=['GET'])
def file_data():
    try:
        all_files = mainBackEnd.get_all_files()
        
        formatted_data = [
            {
                "filename": file_data["filename"],
                "filesize": file_data["filesize"],
                "num_commits": len(mainBackEnd.all_files_history.get(file_path, {}).get("commitDate", [])),
                "cyclomatic_complexity": file_data.get("cycComp", "N/A"),
                "cognitive_complexity": file_data.get("cogComp", "N/A"),
                "lines_of_code": file_data.get("nloc", 0)  # Assuming 'loc' field exists
            }
            for file_path, file_data in all_files.items()
        ]

        return json.dumps({"result": formatted_data})
    except Exception as e:
        print(f"Error in /file_data_tables: {str(e)}")
        return json.dumps({"error": str(e)})


if __name__ == "__main__":
    print("----Starting BackEndServer----")
    mainBackEnd.init_all_files()
    print("----BackEndServer Ready to be used-----")
    server.run(debug=False)
