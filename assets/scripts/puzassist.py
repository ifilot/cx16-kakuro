import sys

def main():
    N = int(sys.argv[1])

    print('0' * N)

    res = []
    res.append('0' * N)
    for i in range(0,N-1):
        n = sys.stdin.read(N+1)
        res.append(n[0:N])

    for i in range(0,N):
        print(' '.join(res[i]))

if __name__ == '__main__':
    main()