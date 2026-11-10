import unittest
from unittest.mock import MagicMock
from BackEndServer import server, mainBackEnd 
import json
import time

class TestFlaskApp(unittest.TestCase):
    def setUp(self):
        self.app = server.test_client()
        self.app.testing = True

    def test_file_names(self):
        # Test the get file_names function
        response = self.app.get('/file_names')
        self.assertEqual(response.status_code, 200)
        data = json.loads(response.data)
        self.assertIn("result", data)
        #Uncomment bellow to check if the files are the correct ones
        #print(f"Files found: {data['result']}")

    def test_file_history(self):
        # Test the get file_history function
        response = self.app.get('/file_history')
        self.assertEqual(response.status_code, 200)
        data = json.loads(response.data)
        self.assertIn("result", data)
        #Uncomment bellow to check if the files are the correct ones
        #print(f"File history found: {data['result']}")

    def test_server_loading_time(self):
        
        start_time = time.time()  
        response = self.app.get('/file_names')
        end_time = time.time()  
        
        
        self.assertEqual(response.status_code, 200)
        data = json.loads(response.data)
        self.assertIn("result", data)

        response_time = end_time - start_time
        print(f"Time taken for /file_names: {response_time:.4f} seconds")


if __name__ == "__main__":
    unittest.main()
