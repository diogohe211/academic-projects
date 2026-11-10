# Bit stream encoding demo
    #
# Developed by Marco Simões on 07/12/2020
# msimoes@dei.uc.pt

import pickle
import os

# função para codificar uma fonte, 
# usando uma tabela de codigos e respetivos comprimentos
def encode(data, table):
    encoded_data = bytearray()
    
    data = list(data) + ['EOF']

    buffer = 0
    size = 0
    for s in data:

        # vou buscar o codigo binario para codificar o simbolo s
        # b -> "comprimento" (num de bits) do cdigo, v -> valor do codigo 
        b, v = table[s] 
       
        # vou adicionar o codigo binario ao buffer. Para isso faço:
        # Shift left do comprimento do simbolo a adicionar e adição do respetivo valor. 
        # Exemplo: buffer tem valor 101, e vou adicionar o valor 10
        # 1) shift left de duas casa (buffer << 2) -> 10100
        # 2) somar o simbolo ao buffer ( buffer + 10 ) -> 10110
        buffer = (buffer << b) + v

        size += b
        # aumento a contagem de bits do buffer

        # se ja tenho bits para retirar um byte completo
        while size >= 8:
            
            # shift right de tudo menos os 8 primeiros bits
            # Exemplo: 1000101010 >> (10-8) -> 10001010
            byte = buffer >> (size - 8)
            
            # adicionar o byte ao array de bytes codificados
            encoded_data.append( byte )

            # remover byte do buffer. 
            # Para isso vou pegar no byte e fazer um shift left de (size - 8) e subtrai-lo o buffer
            # 100010 << (10-8) -> 10001000
            # 10001010 (buffer) - 10001000 -> 10
            buffer = buffer - (byte << (size - 8))
            size -= 8
        
    # se houverem bits no buffer depois de codificar toda a fonte
    if size > 0:
        byte = buffer << (8 - size)
        encoded_data.append( byte )
    

    return encoded_data


def decode(encoded_data, table):
    decoded_data = []

    # inverte tabela: mapeia (bitsize, value) para symbols
    lookup = {(b, v): s for s, (b, v) in table.items()}

    # mascaras para tirar apenas um bit do byte, do mais significativo para o menos
    masks = [     128,      64,      32,      16,       8,       4,       2,       1]
    #        10000000 01000000 00100000 00010000 00001000 00000100 00000010 00000001


    buffer = 0
    size = 0

    # vou percorrer os bytes codificados
    for byte in encoded_data:

        # vou tirar bit a bit, do mais significativo para o menos, e juntando ao buffer
        for m in masks:
            buffer = (buffer << 1) + bool(byte & m)
            size += 1

            # se o buffer de bits e o tamanho existirem na tabela, retiro esse simbolo!
            if (size, buffer) in lookup:
                symbol = lookup[size, buffer]

                if symbol == 'EOF':
                    return decoded_data
                
                decoded_data.append(symbol)

                # reinicio o buffer                
                buffer = 0
                size = 0


# escreve bytrarray para ficheiro
def write_file(filename, data):
    with open(filename, 'wb') as f:
        pickle.dump(data, f)
        f.close()

# le bytearray do ficheiro
def read_file(filename):
    with open(filename, 'rb') as f:
        data = pickle.load(f)
        f.close()
        return data


if __name__ == '__main__':

    # informação a codificar, aqui é um conjunto de caracteres, 
    # mas podia ser um conjunto de valores de pixeis de uma imagem
    # Fonte: 'Hello World' repetido 1000 vezes
    data = 'Hello World'*1000

    # tabela com o simbolo a codificar, devolve o numero de bits e o seu codigo
    # pode ser criada pelo huffman, por exemplo
    table = {
        'H': (4, 11), # 1011
        'e': (4, 14), # 1110
        'l': (2, 1),  # 01
        'o': (2, 0),  # 00
        ' ': (4, 10), # 1010
        'W': (4, 12), # 1100
        'r': (4, 15), # 001
        'd': (4, 13), # 1101
        'EOF': (3, 4) # 100
    }

    #print('Fonte a comprimir: %s' % data)
    
    # codificar e gravar para ficheiro
    encoded_data = encode(data, table)

    # crio um dicionario com a tabela e os dados codificados para guardar no ficheiro
    encoded = {'t':table, 'd':encoded_data}

    # guardo esse dicionario no ficheiro
    write_file('compressed.dat', encoded)

    print('tamanho original: %d, tamanho comprimido: %d' % (len(data), os.path.getsize('compressed.dat')))

    # ler dicionário do ficheiro 
    _encoded = read_file('compressed.dat')

    # separar tabela e dados codificados
    _table = _encoded['t']
    _encoded_data = _encoded['d']

    # descodificar dados
    decoded_data = decode(_encoded_data, _table)

    # validar descodificação
    if data == ''.join(decoded_data):
        print('Decoding successful')
    else:
        print('Decoding unsuccessful')


