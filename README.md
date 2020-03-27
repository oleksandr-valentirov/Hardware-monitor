# Hardware monitor
<p>Libre hardware monitor - open-source software, which gathers data from sensors on your motherboard, CPU, GPU, etc<br>
  and sends it to the local web-server or COM port</p>
<p>Hardware monitor - code for PIC16F18875 which receives tis data from COM port with UART,<br>
  parse it (it is actually a string) and displays it on the character LCD (20x4 and 16x2).</p>
  
### Currently implemented features
<ul>
  <li>parsing of string from UART</li>
  <li>interrupt on connection lost</li>
  <li>1 mode for 20x4 and 1 for 16x2</li>
</ul>

### Should be implemented
<ul>
  <li>at least 2 more modes for 16x2</li>
  <li>second mode for 20x4 with motherboard tmp</li>
  <li>you can add whatever you want, maybe you have more sensors, etc</li>
</ul>

### Libre hardware monitor
<p>Libre hardware monitor can be found on GitHub. It is written on C# and JavaScript.<br>
  I really want to write the same program but with C or C++ or Rust.</p>
<p>Current output is divided by semicolon and has 'E' character as the last character of the data serie.<br>
  <br>
  0-CPU temp, 1-GPU temp, 2-mother temp, 3-max HDD temp, 4-CPU load, 5-GPU load, 6-RAM use, 7-GPU memory use,<br>
  8-maxFAN, 9-minFAN, 10-maxTEMP, 11-minTEMP, 12-manualFAN, 13-manualCOLOR, 14-fanCtrl, 15-colorCtrl,<br>
  16-brightCtrl, 17-LOGinterval, 18-tempSource, 19-AltCPU temp.<br>
  Dummy sample:
  <blockquote>56;67;0;34;0;0;65;0;..;..;..;..;..;..;..;52E</blockquote>
  This data is a sample of real data, but with dots. I never paid attention to coolers.
</p>
