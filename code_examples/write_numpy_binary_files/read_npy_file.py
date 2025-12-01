import numpy as np


# Read the .npy files
A = np.load("matrix_A.npy", allow_pickle=False)
C = np.load("cube_C.npy", allow_pickle=False)

# Print some info and the content of the matrix and cube the files and output read
print()
print(f"Matrix A, data type: {A.dtype}")
print(f"Matrix A, shape: {A.shape}")
print()
print(f"Matrix A:\n {A}")
print()
print()

print(f"Cube C, data type: {C.dtype}")
print(f"Cube C, shape: {C.shape}")
print()
print(f"Cube C:\n {C}")
print()
print()



