; ******************************************************************************
; * © 2008 Microchip Technology Inc.
; *
; SOFTWARE LICENSE AGREEMENT:
; Microchip Technology Incorporated ("Microchip") retains all ownership and 
; intellectual property rights in the code accompanying this message and in all 
; derivatives hereto.  You may use this code, and any derivatives created by 
; any person or entity by or on your behalf, exclusively with Microchip's
; proprietary products.  Your acceptance and/or use of this code constitutes 
; agreement to the terms and conditions of this notice.
;
; CODE ACCOMPANYING THIS MESSAGE IS SUPPLIED BY MICROCHIP "AS IS".  NO 
; WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED 
; TO, IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A 
; PARTICULAR PURPOSE APPLY TO THIS CODE, ITS INTERACTION WITH MICROCHIP'S 
; PRODUCTS, COMBINATION WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
;
; YOU ACKNOWLEDGE AND AGREE THAT, IN NO EVENT, SHALL MICROCHIP BE LIABLE, WHETHER 
; IN CONTRACT, WARRANTY, TORT (INCLUDING NEGLIGENCE OR BREACH OF STATUTORY DUTY), 
; STRICT LIABILITY, INDEMNITY, CONTRIBUTION, OR OTHERWISE, FOR ANY INDIRECT, SPECIAL, 
; PUNITIVE, EXEMPLARY, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, FOR COST OR EXPENSE OF 
; ANY KIND WHATSOEVER RELATED TO THE CODE, HOWSOEVER CAUSED, EVEN IF MICROCHIP HAS BEEN 
; ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT 
; ALLOWABLE BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO 
; THIS CODE, SHALL NOT EXCEED THE PRICE YOU PAID DIRECTLY TO MICROCHIP SPECIFICALLY TO 
; HAVE THIS CODE DEVELOPED.
;
; You agree that you are solely responsible for testing the code and 
; determining its suitability.  Microchip has no obligation to modify, test, 
; certify, or support the code.
;
; *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.equ __p33EP64GS805, 1
.include "p33EP64GS805.inc"

.include "dspcommon.inc"

; Maximum DAC reference for current mode control
; 28A / 125 = .224A on secondary side
; where 28A will be the maximum current
; .224 * 6 ohm * .9297  = 1.25 V
; where .9297 is the gain of U9:D
; CMREF = (1.25 * 1024) / (AVdd / 2) = 776 
;.equ SinglePhaseMaxDC, 776

; Minimum DAC reference for current mode control
;.equ SinglePhaseMinDC, 10

; Maximum Duty cycle for voltage mode control
.equ MultiPhaseMaxDC, 1224

; Minimum Duty cycle for voltage mode control
.equ MultiPhaseMinDC, 72


        .equ    offsetabcCoefficients, 0
        .equ    offsetcontrolHistory, 2
        .equ    offsetcontrolOutput, 4
        .equ    offsetmeasuredOutput, 6
        .equ    offsetcontrolReference, 8
.data
.text

.global _Multiphase_Current
.global _PCB_Temperature

.global __ADCAN0Interrupt
.global __ADCAN1Interrupt
.global __ADCAN2Interrupt
.global __ADCAN3Interrupt
.global __ADCAN4Interrupt

__ADCAN0Interrupt:
; This ADC Interrupt is used to sense the 12V output and to transmit the output to the Primary side
; Total number of instructions: 8 cycles for interrupt latency and return + 10 cycles in ISR
; Total time 18 cycles at 25ns = 450ns
	push w0

	mov ADCBUF0, w0						; Measure Multiphase Current. Over-current fault checked in fault routine
	mov w0, _Buck_Input
   
	bclr 	ADSTATL, #0					; Clear Pair 2 conversion status bit 
	bclr    IFS6, #14					; Clear  interrupt Flag
	
    pop w0
    retfie			 					; Return from interrupt	
	
	
__ADCAN1Interrupt:
;This ADC Interrupt calls the PID and updates duty cycle for the multiphase buck converters
; Total number of instructions: 8 cycles for interrupt latency and return + 55 instruction cycles in ISR
; Total time 63 instruction cycles at 25ns = 1.575us
	push.s
	push w4
	push w5
	push w8
	push w10
	PUSH CORCON								; Prepare CORCON for fractional computation.

    mov #_MultiPhaseVoltagePID, w0			; Load w0 with address of PID structure
    mov ADCBUF1, w1							; Load w1 with the 3.3V sense voltage
    sl  w1, #3, w1							; Shift left by 5 for Q15 format for PID
	
    mov w1, [w0+#offsetmeasuredOutput]		; Pass the measured voltage to the PID structure

	;Execute PID routine. (See PID.s for full description of PID library)

    fractsetup      w8

    mov [w0 + #offsetabcCoefficients], w8    ; w8 = Base Address of _abcCoefficients array [(Kp+Ki+Kd), -(Kp+2Kd), Kd]
    mov [w0 + #offsetcontrolHistory], w10    ; w10 = Address of _ControlHistory array (state/delay line)

    mov [w0 + #offsetcontrolOutput], w1
    mov [w0 + #offsetmeasuredOutput], w2
    mov [w0 + #offsetcontrolReference], w3

    ; Calculate most recent error with saturation, no limit checking required
    lac     w3, a                   ; A = tPID.controlReference
    lac     w2, b                   ; B = tPID.MeasuredOutput
    sub     a                       ; A = tPID.controlReference - tPID.measuredOutput
    sac.r   a, [w10]                ; tPID.ControlHistory[n] = Sat(Rnd(A))

    ; Calculate PID Control Output
    clr     a, [w8]+=2, w4, [w10]+=2, w5            ; w4 = (Kp+Ki+Kd), w5 = _ControlHistory[n]
    lac     w1, a                                   ; A = ControlOutput[n-1]
    mac     w4*w5, a, [w8]+=2, w4, [w10]+=2, w5     ; A += (Kp+Ki+Kd) * _ControlHistory[n]
                                                    ; w4 = -(Kp+2Kd), w5 = _ControlHistory[n-1]
    mac     w4*w5, a, [w8], w4, [w10]-=2, w5        ; A += -(Kp+2Kd) * _ControlHistory[n-1]
                                                    ; w4 = Kd, w5 = _ControlHistory[n-2]
    mac     w4*w5, a, [w10]+=2, w5                  ; A += Kd * _ControlHistory[n-2]
                                                    ; w5 = _ControlHistory[n-1]
                                                    ; w10 = &_ControlHistory[n-2]
    sac.r   a, w1                                   ; ControlOutput[n] = Sat(Rnd(A))
												
    ;Update the error history on the delay line
    mov     w5, [w10]               ; _ControlHistory[n-2] = _ControlHistory[n-1]
    mov     [w10 + #-4], w5         ; _ControlHistory[n-1] = ControlHistory[n]
    mov     w5, [--w10]

	;  nd of PID routine

	;Clamp PID output to 10bits. The PID coeeficients were scaled by a factor of 32 for Q15 format, which means 
	;the PID control output has to be scaled to the same extent. By clamping by 0x3FF we are essientially
	;dividing the max output by 32. (32767 % 32) = 1024 = 0x3FF
    mov.w #0x00, w2				
	cpsgt w1, w2
	mov.w w2, w1
	mov.w #0x3FF, w2
	cpslt w1, w2
	mov.w w2, w1

	mov.w w1, [w0+#offsetcontrolOutput]	; Update control output

	sl w1, #5, w4						; Shift left to back to Q15 format
	bclr CORCON, #0
		
	mov #1923, w5
	mpy w4*w5, a						; Multiply the control output with the Period
										; This gives us a value for the duty cycle which is now
										; between 0 -> 1923

	sac.r a, w0							; Saturate the accumulator

    mov.w #MultiPhaseMinDC, w1			; Saturate to minimum allowed duty cycle
	cpsgt w0, w1
	mov.w w1, w0
	mov.w #MultiPhaseMaxDC, w1			; Saturate to maximum allowed duty cycle
	cpslt w0, w1
	mov.w w1, w0
	
	mov.w w0, MDC, 						; Update Master Duty Cycle
    mov.w w0, TRIG1						; Update trigger

        bclr	ADSTATL,#1					; Clear Pair 1 conversion status bit
   	bclr    IFS6,#15					; Clear Pair 1 interrupt Flag

	pop CORCON							; Restore CORCON and other working registers
	pop w10
	pop w8
	pop w5
    pop w4
    pop.s
	retfie			 					; Return from interrupt



__ADCAN2Interrupt:
; This ADC interrupt is used for the PCB over temperature
; Total number of instructions: 8 cycles for interrupt latency and return + 6 cycles in ISR
; Total time 14 cycles at 25ns = 350ns 
    push w0

    mov ADCBUF2, w0						; Load w0 with Temperature sample and load variable PCB_Temperature
    mov w0, _Multiphase_Current			; This will be compared in the fault check routine with the 
    									; defined max temperature (TEMPERATURE_MAX). See main.c for detailed calculation of max temp 

    bclr	ADSTATL,#2					; Clear Pair 3 conversion status bit
    bclr   IFS7 , #0				; Clear Pair 3 interrupt Flag

    pop w0
    retfie					 			; Return from interrupt

	
	
__ADCAN3Interrupt:
; This ADC interrupt is used for the PCB over temperature
; Total number of instructions: 8 cycles for interrupt latency and return + 6 cycles in ISR
; Total time 14 cycles at 25ns = 350ns 
    push w0

    mov ADCBUF3, w0						; Load w0 with Temperature sample and load variable PCB_Temperature
    mov w0, _PCB_Temperature			; This will be compared in the fault check routine with the 
    									; defined max temperature (TEMPERATURE_MAX). See main.c for detailed calculation of max temp 

    bclr	ADSTATL,#3					; Clear Pair 3 conversion status bit
    bclr   IFS7 , #1				; Clear Pair 3 interrupt Flag

    pop w0
    retfie					 			; Return from interrupt

    
    .end