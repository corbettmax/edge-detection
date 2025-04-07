#include <mpi.h>
#include <cmath>
#include <utility>
#include <queue>
#include "algorithms.h"
#include "kernels.h"
#include <iostream>
#include <functional>
#include <QImage>

int main(int argc, char *argv[])
{
    // Initialize MPI
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Start the timer
    double start_time = MPI_Wtime();

    // EDIT THESE FOR DESIRED ALGORITHM AND IMAGE
    QString imagePath = "../img/G20RioSummit.jpg"; // Path to image
    QString computation_mode = "scharr";      // Options: canny, sobel, prewitt, roberts, scharr

    // Load the image
    QImage original(imagePath);
    if (original.isNull()) {
        if (rank == 0) {
            printf("Failed to load image: %s\n", imagePath.toStdString().c_str());
        }
        MPI_Finalize();
        return -1;
    }

    // Convert the image to grayscale
    QImage grayscale = original.convertToFormat(QImage::Format_Grayscale8);

    // Divide the image rows among processes
    int rows_per_process = grayscale.height() / size;
    int start_row = rank * rows_per_process;
    int end_row = (rank == size - 1) ? grayscale.height() : start_row + rows_per_process;

    // Allocate buffers for halo rows
    QVector<quint8> top_halo(grayscale.bytesPerLine());
    QVector<quint8> bottom_halo(grayscale.bytesPerLine());
    QVector<quint8> send_top(grayscale.bytesPerLine());
    QVector<quint8> send_bottom(grayscale.bytesPerLine());

    // Prepare send buffers
    if (start_row > 0) {
        memcpy(send_top.data(), grayscale.constScanLine(start_row), grayscale.bytesPerLine());
    }
    if (end_row < grayscale.height()) {
        memcpy(send_bottom.data(), grayscale.constScanLine(end_row - 1), grayscale.bytesPerLine());
    }

    // halo exchange
    double halo_start_time = MPI_Wtime();
    MPI_Request requests[4];
    if (rank > 0) {
        MPI_Isend(send_top.data(), grayscale.bytesPerLine(), MPI_BYTE, rank - 1, 0, MPI_COMM_WORLD, &requests[0]);
        MPI_Irecv(top_halo.data(), grayscale.bytesPerLine(), MPI_BYTE, rank - 1, 1, MPI_COMM_WORLD, &requests[1]);
    }
    if (rank < size - 1) {
        MPI_Isend(send_bottom.data(), grayscale.bytesPerLine(), MPI_BYTE, rank + 1, 1, MPI_COMM_WORLD, &requests[2]);
        MPI_Irecv(bottom_halo.data(), grayscale.bytesPerLine(), MPI_BYTE, rank + 1, 0, MPI_COMM_WORLD, &requests[3]);
    }

    // Wait for halo exchange to complete
    if (rank > 0) {
        MPI_Wait(&requests[0], MPI_STATUS_IGNORE);
        MPI_Wait(&requests[1], MPI_STATUS_IGNORE);
    }
    if (rank < size - 1) {
        MPI_Wait(&requests[2], MPI_STATUS_IGNORE);
        MPI_Wait(&requests[3], MPI_STATUS_IGNORE);
    }
    double halo_end_time = MPI_Wtime();

    // Prepare local image with halo rows
    QImage local_input(grayscale.width(), rows_per_process + 2, grayscale.format());
    if (rank > 0) {
        memcpy(local_input.scanLine(0), top_halo.data(), grayscale.bytesPerLine());
    }
    for (int y = 0; y < rows_per_process -1; y++) {
        memcpy(local_input.scanLine(y + 1), grayscale.constScanLine(start_row + y), grayscale.bytesPerLine());
    }
    if (rank < size - 1) {
        memcpy(local_input.scanLine(rows_per_process + 1), bottom_halo.data(), grayscale.bytesPerLine());
    }

    // Perform edge detection based on the computation mode
    double computation_start_time = MPI_Wtime();
    QImage local_result(grayscale.width(), rows_per_process, grayscale.format());
    if (computation_mode == "canny") {
        local_result = canny(local_input, 1, 40, 120);
    } else if (computation_mode == "sobel") {
        local_result = sobel(local_input);
    } else if (computation_mode == "prewitt") {
        local_result = prewitt(local_input);
    } else if (computation_mode == "roberts") {
        local_result = roberts(local_input);
    } else if (computation_mode == "scharr") {
        local_result = scharr(local_input);
    } else {
        if (rank == 0) {
            printf("Invalid computation mode. Use canny, sobel, prewitt, roberts, or scharr.\n");
        }
        MPI_Finalize();
        return -1;
    }
    double computation_end_time = MPI_Wtime();

    // Gather results from all processes
    double gather_start_time = MPI_Wtime();
    QVector<quint8> gathered_data;
    if (rank == 0) {
        gathered_data.resize(grayscale.height() * grayscale.bytesPerLine());
    }
    MPI_Gather(local_result.bits(), local_result.sizeInBytes(), MPI_BYTE,
           rank == 0 ? gathered_data.data() : nullptr, local_result.sizeInBytes(), MPI_BYTE, 0, MPI_COMM_WORLD);
    double gather_end_time = MPI_Wtime();

    // If rank 0, assemble the final image
    if (rank == 0) {
        QImage result(grayscale.size(), grayscale.format());
        memcpy(result.bits(), gathered_data.data(), gathered_data.size());
        result.save("../img/result.jpg");
        printf("Result saved as result.jpg\n");
    }

    // End the timer
    double end_time = MPI_Wtime();

    // Print benchmarking results
    if (rank == 0) {
        printf("Algorithm: %s\n", computation_mode.toStdString().c_str());
        printf("Total execution time: %.3f ms\n", (end_time - start_time)*1000);
        printf("Halo exchange time: %.3f ms\n", (halo_end_time - halo_start_time)*1000);
        printf("Computation time: %.3f ms\n", (computation_end_time - computation_start_time)*1000);
        printf("Gather time: %.3f ms\n", (gather_end_time - gather_start_time)*1000);
    }

    // Finalize MPI
    MPI_Finalize();
    return 0;
}
