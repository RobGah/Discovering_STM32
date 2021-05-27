Exercise11p2 - USART Flow Control Notes

This is a conceptually difficult topic so I made notes. 

Basic USART has fundamentally 2 operations:
1. putchar() writes to the TX register
2. getchar() reads the RX register

Two big flaws with this: 
-Code slows to the baud rate if the tx register is not empty, we poll until tx is empty.
-Overflow of rx is not handled and results in lost data.

Of the two, overflow is the most serious because you lose data. 

Strategies to overcome this:
1. Make tx and rx queues (circular buffers)
    -literally data buffers to hold pending tx chars and recieved rx chars
    -Circular buffers have two pointers (author uses 2 index positions instead)
    -"pointers" are current thing to read and next empty cell to write.
    read "chases" write around the buffer. Circular nature of bufffer refers to fact that the
    buffers ends are tied together - you go back to 0 if you hit the end.
    -youtube has a good explanation of circular buffers.  
-putchar() then instead "enqueues" a char in the tx queue for sending
-getchar() then instead "dequeues" a char from the rx queue each time we read.
2. Create interrupt conditions (and an IRQHandler function) to handle two the two conditions we 
get from USART status bits RXNE and TXE aka recieve register not empty and transmit register empty
-on TXE being true, we dequeue a char byte from the tx queue and feed it into the tx register
    IF the queue is empty (corner case), we disable the interrupt and let the application code
    re-enable the interrupt when putchar() is called, with variable TXprimed being a flag between
    the USART interrupt handler and the application.
-on RXNE being true we enqueue the char byte in the rx register into the RX queue struct
    HOWEVER we can still overflow if the rx queue is full and we set a flag rxoverflow.

The application (aka getchar() and putchar()) now handles:
-The enqueueing of tx bytes into the tx queue
-The dequeueing of rx bytes from the rx queue to be read by the application

The interrupt handler handles:
-The dequeueing of bytes from the tx queue circular buffer to be fed to the tx register for sending
    -The interrupt condition where the tx register is empty and needs feeding.
-The enqueuing of rx bytes into the rx queue for eventual reading by the application
    -The interrupt condition where the rx register is full and the byte needs to be enqueued 
    into the rx queue circular buffer.

#1 + #2 alleviate but don't eliminate the possiblity of data loss. We need hw flow control for that.
-HW flow control consists of physical lines RTS and CTS. (Request and Clear to Send)
-nRTS signals that its ok to send data and is connected to nCTS on the opposing side.
-each USART device then has an nRTS that it can signal a readiness to recieve data. 
-Can hold pin high to stop the flow of data
-In practice need to define a threshold or "high water" mark for data stoppage because the flow 
of data might be delayed by a few bytes. This is literally some number amount of open slots in the 
rx queue buffer at which we think overflow will not occur when RTS is suddenly shifted high and
bytes keep rolling in due to the delay between the other USART receiving the RTS signal and
the stoppage of bytes flowing in. Author uses QUEUE_SIZE - 6. He doesn't specify queue size. 


