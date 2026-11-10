import matplotlib.image as mpimg
import numpy as np
from huffmancodec import HuffmanCodec
import pickle
import math
import os


# le o ficheiro de imagem, retorna a array de valores do ficheiro e as dimensoes da imagem
def parse(file_name):
    file = mpimg.imread(file_name)
    return file.astype('u8'), file.shape


# aplica run length encoding na array
def run_len(arr):
    print("Iniciando run length")
    new = list()
    x = 0
    last_element = None
    # percorre elementos da array, e os guarda em tuplos (a, b)
    # onde a = elemento e b = numero de apariçoes em sequencia
    for element in arr:
        if last_element is None:
            last_element = element
        elif last_element != element or x >= 127:
            new.append((last_element, x))
            last_element = element
            x = 0
        x += 1
    new.append((last_element, x))
    print("Finalizando run length")
    return new


# le os valores dos codigos em huffman e os escreve em uma array de bytes
def encode(data, table):
    print("Inicio Encoding")
    encoded = bytearray()
    data = list(data)
    buffer = ""
    size = 0
    for s in data:
        b, v = table[s]
        buffer += np.binary_repr(v, width=b)
        size += b
        # se o buffer tiver pelo menos 8 bits, escreve o byte no array
        while size >= 8:
            byte = int(buffer[:8], 2)
            encoded.append(byte)
            size -= 8
            buffer = buffer[8:]
    # se sobrar bits no buffer, escreve os bits na array, preenchendo o final do valor com zeros se necessario.
    if size > 0:
        byte = int("{}{}".format(buffer, '0' * (8 - size)), 2)
        encoded.append(byte)
    print("Fim Encoding")
    return encoded


# descodifica a array de bytes para uma array com os valores referentes a huffman table
def decode(encoded_data, table):
    print("Iniciando decoding")
    decoded_data = []
    lookup = {(b, v): s for s, (b, v) in table.items()}
    masks = [128, 64, 32, 16, 8, 4, 2, 1]
    buffer = 0
    size = 0
    for byte in encoded_data:
        # adiciona bit a bit no buffer, verificando a cada passo se o buffer existe na huffman table
        for m in masks:
            buffer = (buffer << 1) + bool(byte & m)
            size += 1
            # se existir, adiciona o simbolo referente ao buffer na array de output e limpa o buffer
            if (size, buffer) in lookup:
                symbol = lookup[size, buffer]
                decoded_data.append(symbol)
                buffer = 0
                size = 0
    print("Finalizando decoding")
    # devolve a array descodificada
    return decoded_data


# escreve o dicionario no ficheiro binario
def write(file_name, data):
    print("Inicio Escrita")
    with open(file_name, 'wb') as f:
        pickle.dump(data, f)
        f.close()
    print("Fim Escrita")


# le o dicionario do ficheiro binario
def read(file_name):
    with open(file_name, 'rb') as f:
        data = pickle.load(f)
        f.close()
        return data


# faz o decode no run length encoding
def decode_rl(arr, size):
    output = []
    for element in arr:
        # do conjunto (a, b) lido, escreve o elemento a b vezes
        for k in range(element[1]):
            output.append(element[0])
            if len(output) == np.prod(size):
                # retorna a array final, com as dimensoes da imagem original
                return np.array(output, dtype='u8').reshape(size)
    return np.array(output, dtype='u8').reshape(size)


# chama as funçoes necessarias para fazer o encoding
def encoding(array, size, output_f):
    to_compress = array.flatten()
    r_l = run_len(to_compress)
    codex = HuffmanCodec.from_data(r_l)
    table = codex.get_code_table()
    encoded = encode(r_l, table)
    # cria o dicionario com as dimensões da imagem original, com a tabela huffman e com a array de bytes
    end_result = {'s': size, 't': table, 'd': encoded}
    write(output_f, end_result)


# chama as funçoes necessarias para fazer o decoding
def decoding(array):
    to_decode_table = array['t']
    to_decode_data = array['d']
    size = array['s']
    decoded_data = decode(to_decode_data, to_decode_table)
    decoded = decode_rl(decoded_data, size)
    return decoded


# faz a compressão e a descompressão da imagem, e verifica se a descompressão foi feita corretamente
def compress(input_f, output_f):
    print("------------------------------\nComprimindo " + input_f + "\n------------------------------")
    to_compress_unflattened, shape = parse(input_f)
    encoding(to_compress_unflattened, shape, output_f)
    to_decode = read(output_f)
    decoded = decoding(to_decode)
    print("Tamanho original: {}\nTamanho comprimido: {}".format(os.path.getsize(input_f), os.path.getsize(output_f)))
    print("Decode funcional? {}".format(np.array_equal(decoded, to_compress_unflattened)))


if __name__ == "__main__":
    input_file = input("Digite o nome do ficheiro a ser comprimido: ")
    output_file = input("Digite o nome do ficheiro de saida: ")
    compress(input_file, output_file)
    