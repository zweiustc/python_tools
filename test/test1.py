class Solution(object):
    def __init__(self, *args, **kwargs):
        super(Solution, self).__init__(*args, **kwargs)
        print "solution init"

    def convertToTitle(self, n):
        temp = n
        result = []
        while(temp >0):
            yushu = temp % 26
            symbol = chr(64 + yushu)
            temp = temp/26
            result.append(symbol)
        result = result[::-1]
        test = ''
        result = test.join(result)
        if isinstance(result, list):
            print "it is list"
        else:
            print "it is string"
        return result

if __name__ == "__main__":
    print "test"
    test = Solution()
    n = 28
    result = test.convertToTitle(n)
    print "%s is %s" % (n, result)
    
