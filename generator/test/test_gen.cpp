#include <iostream>
#include "../../parser/include/parser.h"
#include "../include/generator.h"

int main() {
    std::string source_code = R"(

class MergeSortExample {

    public void test(){
        for(int i = 0; i < 10; i++) System.out.println(i);
    }

    public static void main(String[] args) {
        int[] arr;
        int size;
        MergeSort ms;

        // Initialize the array and the MergeSort object
        size = 6;

        arr = new int[size];
        arr[0] = 38;
        arr[1] = 27;
        arr[2] = 43;
        arr[3] = 3;
        arr[4] = 9;
        arr[5] = 82;

        ms = new MergeSort();
        arr = ms.sort(arr);

        // Print the sorted array
        int i;
        i = 0;
        while (i < size) {
            System.out.println(arr[i]);
            i = i + 1;
        }
    }
}

class Merger {

    public int[] merge(int[] left, int[] right) {
        int[] result;
        int i;
        int j;
        int k;

        result = new int[left.length + right.length];
        i = 0; // Index for left array
        j = 0; // Index for right array
        k = 0; // Index for result array

        // Merge while there are elements in both arrays
        while (i < left.length && j < right.length) {
            if (left[i] < right[j]) {
                result[k] = left[i];
                i = i + 1;
            } else {
                result[k] = right[j];
                j = j + 1;
            }
            k = k + 1;
        }

        // Copy any remaining elements from the left array
        while (i < left.length) {
            result[k] = left[i];
            i = i + 1;
            k = k + 1;
        }

        // Copy any remaining elements from the right array
        while (j < right.length) {
            result[k] = right[j];
            j = j + 1;
            k = k + 1;
        }

        return result;
    }

}

class MergeSort extends Merger {
    public int[] sort(int[] arr) {
        if (arr.length <= 1) {
            return arr; // Base case for recursion
        }

        // Split the array into two halves
        int mid;
        mid = arr.length / 2;
        int[] left;
        int[] right;

        left = this.subArray(arr, 0, mid);
        right = this.subArray(arr, mid, arr.length);

        // Recursive sorting
        left = this.sort(left);
        right = this.sort(right);

        // Merge the sorted halves
        return this.merge(left, right);
    }

    public int[] subArray(int[] arr, int start, int end) {
        int[] subArr;
        int i;

        subArr = new int[end - start];
        i = 0;
        while (i < subArr.length) {
            subArr[i] = arr[start + i];
            i = i + 1;
        }
        return subArr;
    }
}
    )";

    /// Lexer
    /*auto tokens = tokenize(source_code);
    for (const auto &token: tokens) {
        if (token.type == TokenType::WHITESPACE) continue;

        std::cout << token << std::endl;
    }*/

    printf("-------\n");

    /// Parser and Semantic Analyser
    auto project = parse(source_code);
    std::cout << project << std::endl;

    printf("-------\n");

    /// Code Generator
    generate(&project);

    return 0;
}
