using Microsoft.Win32;
using System;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.NetworkInformation;
using System.Net.Sockets;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace WebSocketServer
{
    class Program

    {
        [DllImport("kernel32.dll", EntryPoint = "LoadLibrary")]
        private extern static IntPtr LoadLibrary(string librayName);

        [DllImport("kernel32.dll", EntryPoint = "GetProcAddress", CharSet = CharSet.Ansi)]
        private extern static IntPtr GetProcAddress(IntPtr hwnd, string procedureName);

        [DllImport("kernel32.dll", EntryPoint = "FreeLibrary")]
        private extern static bool FreeLibrary(IntPtr hModule);


        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void OnTest1();
        private static OnTest1 func_OnTest1/*dll에서 C#용으로 쓰는 함수*/;

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void shutdownWAS();
        private static shutdownWAS func_shutdownWAS;

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate IntPtr _GetAdapterDescription(int[] size, int index, int[] ip);
        private static _GetAdapterDescription func_GetAdapterDescription;

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate int _SetAdapter(string AdapterDescription);
        private static _SetAdapter func_SetAdapter;

        static bool quit = false;

        static String GetAdapterDescription(ref int index, ref int ip_address)
        {
            int[] _nn = new int[1];
            int[] _ip = new int[1];

            if (func_GetAdapterDescription == null)
            {
                Load("was_dll");
            }

            IntPtr ptr = func_GetAdapterDescription(_nn, index, _ip);

            if (index == 0)
            {
                index = _nn[0];
            }

            ip_address = _ip[0];

            return String.Copy(Marshal.PtrToStringAnsi(ptr));

        }

        static string ToAddr(int address)
        {
            return new IPAddress(BitConverter.GetBytes(address)).ToString();
        }

        static void FuncA()
        {
            func_OnTest1(); Console.WriteLine("END Server");
        }
        static void Main(string[] args)
        {
            RegistryKey reg = Registry.LocalMachine;
            reg = reg.OpenSubKey("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\WinPcapInst");

            if (reg == null)
            {
                Console.WriteLine("WinPCap이 설치되어있지 않는것같습니다. WinPCap을 설치해주세요.");
                Console.WriteLine("URL : https://www.winpcap.org/install/bin/WinPcap_4_1_3.exe");
                Console.ReadLine();
                return;
            }
            else
                reg.Close();


            int i;
            String[] AdapterDescription_Name = new String[30];
            int[] AdapterIPAdress = new int[30];
            int Adapter_Number = 0;
            int k;

            if (func_GetAdapterDescription == null) // c++ 함수 load
            {
                Load("was_dll");
            }

            // PCap Adapter 선택문장
            AdapterDescription_Name[0] = GetAdapterDescription(ref Adapter_Number, ref AdapterIPAdress[0]);
            for (i = 1, k = 2; i < Adapter_Number; i++, k++)
                AdapterDescription_Name[i] = GetAdapterDescription(ref k, ref AdapterIPAdress[i]);
            for (i = 0; i < Adapter_Number; i++)
                Console.WriteLine("{0}. {1}\t(IP Address : {2})" , (i + 1), AdapterDescription_Name[i], ToAddr(AdapterIPAdress[i]));
            if (Adapter_Number > 1)
            {
                Console.Write("어댑터 선택 : ");
                i = int.Parse(Console.ReadLine());
            }
            else
                i = 1;
            Console.WriteLine("{0}어댑터를 PCap출력 인터페이스로 설정합니다.", AdapterDescription_Name[i - 1]);
            func_SetAdapter(AdapterDescription_Name[i - 1]);

            Task taskA = new Task(FuncA);
            taskA.Start();

            byte[] _pingPong_Message = new byte[4] { 1, 0, 1, 1 };
            AsynchronousClient client = new AsynchronousClient("127.0.0.1",3001);
            client.startClient();
            //client.setCallBack_PingPong(PingPong_Action);


            if (!client.checkConnection())
            {
                Console.WriteLine("연결이 필요합니다. {0}", client.Connected);
                return;
            }
            else
            {
                Console.WriteLine("서버응답을 테스트합니다.");
                client.Send(_pingPong_Message); // 첫번째 인자 : 전달할 배열, 두번째 인자(생략가능) : blocking, 세번째 인자 : asynchronousc모드이기때문에 수신데이터가 전달되기전에 종료되는 상황을 대비하기 위한 것
                client.Receive(); // 첫번째 인자 : blocking, ReceiveCallback함수 내용중 446줄이 buffer이고(길이는 buffer의 크기만큼 할당됨), 442줄 if문안에서 Access하면됨
            }

            Thread.Sleep(500); // Wait the WebSocketServer response 
            Console.Write("자동으로 계속 확인하고, 문제 발생시 서버를 재부팅 시키는 모드로 작동시킬까요?(Yes = Y, No = Any Key) : ");
            if (Console.ReadKey().Key == ConsoleKey.Y)
            {
                while (true)
                {
                    if (!client.checkConnection())
                    {
                        while (client.isConnectionProcessOK)
                        {
                            client.endClient();
                            Thread.Sleep(10);
                        }
                        client.startClient();
                    }
                    else
                    {
                        if (client.noPing > 50)
                        {
                            // Server Reboot
                            Console.WriteLine("WAS Server가 제대로 동작하지 않고 있으므로 재부팅합니다.");
                            func_shutdownWAS();
                            taskA.Wait();
                            taskA.Start();
                            Thread.Sleep(10);
                        }

                        client.Send(_pingPong_Message);
                    }

                    Thread.Sleep(750);
                }
            }
            else
            {
                Console.WriteLine("\n\nSilent Mode로 동작합니다.");
                while (true)
                    Thread.Sleep(999999999);
            }
        }

        public static void PingPong_Action(Boolean isPingPong)
        {
            if (isPingPong)
            {

            }
            else
            {

            }

        }

        static IntPtr hModule;
        public static void Load(string dllFileName)
        {
            // Get the byte[] of the DLL
            byte[] ba = null;
            string resource = System.Reflection.Assembly.GetEntryAssembly().GetName().Name + ".Resources." + dllFileName + ".dll";
            Assembly curAsm = Assembly.GetExecutingAssembly();
            using (Stream stm = curAsm.GetManifestResourceStream(resource))
            {
                ba = new byte[(int)stm.Length];
                stm.Read(ba, 0, (int)stm.Length);
            }

            bool fileOk = false;
            string tempFile = "";

            using (SHA1CryptoServiceProvider sha1 = new SHA1CryptoServiceProvider())
            {
                // Get the hash value of the Embedded DLL
                string fileHash = BitConverter.ToString(sha1.ComputeHash(ba)).Replace("-", string.Empty);

                // The full path of the DLL that will be saved
                tempFile = Path.GetTempPath() + dllFileName + ".dll";

                // Check if the DLL is already existed or not?
                if (File.Exists(tempFile))
                {
                    // Get the file hash value of the existed DLL
                    byte[] bb = File.ReadAllBytes(tempFile);
                    string fileHash2 = BitConverter.ToString(sha1.ComputeHash(bb)).Replace("-", string.Empty);

                    // Compare the existed DLL with the Embedded DLL
                    if (fileHash == fileHash2)
                    {
                        // Same file
                        fileOk = true;
                    }
                    else
                    {
                        // Not same
                        fileOk = false;
                    }
                }
                else
                {
                    // The DLL is not existed yet
                    fileOk = false;
                }
            }

            // Create the file on disk
            if (!fileOk)
            {
                System.IO.File.WriteAllBytes(tempFile, ba);
            }

            hModule = LoadLibrary(tempFile);

            if (hModule == IntPtr.Zero)
            {
                Exception e = new Win32Exception();
                throw new DllNotFoundException(dllFileName + "을 찾을 수 없습니다. 잠시후 다시 해주세요.\n");
            }

            // allocating all functions...
            IntPtr pFuncAddr = IntPtr.Zero;


            pFuncAddr = GetProcAddress(hModule, "OnTest1"/*dll 함수 이름*/); // 메모리에 올라간 dll의 내용중 OnTest1이라는 함수의 주소를 찾는다.
            func_OnTest1/*dll에서 C#용으로 쓰는 함수*/ = (OnTest1/*dll 함수 이름*/)Marshal.GetDelegateForFunctionPointer(pFuncAddr, typeof(OnTest1/*dll 함수 이름*/)); // 찾은 함수의 주소를 포인팅한다.

            pFuncAddr = GetProcAddress(hModule, "shutdownWAS"/*dll 함수 이름*/);
            func_shutdownWAS/*dll에서 C#용으로 쓰는 함수*/ = (shutdownWAS/*dll 함수 이름*/)Marshal.GetDelegateForFunctionPointer(pFuncAddr, typeof(shutdownWAS/*dll 함수 이름*/));

            pFuncAddr = GetProcAddress(hModule, "_GetAdapterDescription"/*dll 함수 이름*/);
            func_GetAdapterDescription/*dll에서 C#용으로 쓰는 함수*/ = (_GetAdapterDescription/*dll 함수 이름*/)Marshal.GetDelegateForFunctionPointer(pFuncAddr, typeof(_GetAdapterDescription/*dll 함수 이름*/));


            pFuncAddr = GetProcAddress(hModule, "_SetAdapter"/*dll 함수 이름*/);
            func_SetAdapter/*dll에서 C#용으로 쓰는 함수*/ = (_SetAdapter/*dll 함수 이름*/)Marshal.GetDelegateForFunctionPointer(pFuncAddr, typeof(_SetAdapter/*dll 함수 이름*/));

            // load는 더이상 하지 않아도 된다. 다만 함수를 Delegate해준것에 대해 위와 같이 포인팅작업을 해주어야한다.
            // pFuncAddr = GetProcAddress(hModule, dll 함수이름);
            // add = ... 위와 같음, 그리고 30~32줄처럼 해주어야함. 반환값이나 전달인자는 C/C++에서 정의한대로 적어주면됨.
        }

        /*~Program()
         {
            FreeLibrary(hModule);
         }*/
    }



    public class AsynchronousClient
    {
        // State object for receiving data from remote device.  
        public class StateObject
        {
            // Client socket.  
            public Socket workSocket = null;
            // Size of receive buffer.  
            public const int BufferSize = 256;
            // Receive buffer.  
            public byte[] buffer = new byte[BufferSize];
            // Received data string.  
            public StringBuilder sb = new StringBuilder();
        }

        // string str = Encoding.Default.GetString(StrByte);
        // byte[] StrByte = Encoding.UTF8.GetBytes(str);


        // ManualResetEvent instances signal completion.  
        private static ManualResetEvent connectDone =
            new ManualResetEvent(false);
        private static ManualResetEvent disconnectDone =
            new ManualResetEvent(false);
        private static ManualResetEvent sendDone =
            new ManualResetEvent(false);
        private static ManualResetEvent receiveDone =
            new ManualResetEvent(false);

        //public delegate void ReceiveCallBack_function(byte[] buf);
        //private ReceiveCallBack_function Receive_CallBack;
        public delegate void ReceiveCallBack_function(byte[] buffer);
        private static ReceiveCallBack_function Receive_CallBack;

        private int _port;
        private IPAddress _ip;
        private IPEndPoint remoteEP;
        private Socket client;
        private Boolean isConnectionProcess = false;

        private static UInt64 _noPingPong_Count = 0;

        public AsynchronousClient()
        {
            _port = 8080;
            _ip = IPAddress.Loopback;
        }
        public AsynchronousClient(String ip, int port)
        {
            _port = port;
            _ip = IPAddress.Parse(ip);
        }

        public void SetReceiveCallBack_Func(ReceiveCallBack_function func)
        {
            Receive_CallBack = func;
        }

        public void startClient()
        {
            try
            {
                remoteEP = new IPEndPoint(_ip, _port);
                client = new Socket(_ip.AddressFamily, SocketType.Stream, ProtocolType.Tcp);
                client.BeginConnect(remoteEP, new AsyncCallback(ConnectCallback), client);
                connectDone.WaitOne(); // wait to complete the connection

                isConnectionProcess = true;

            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }

        public void endClient(Boolean Tell=false)
        {
            if (client.Connected)
            {
                client.Shutdown(SocketShutdown.Both);
            }
            else
                return;

            client.BeginDisconnect(true, new AsyncCallback(DisconnectCallback), client);

            // Wait for the disconnect to complete.
            disconnectDone.WaitOne();
            if (client.Connected)
                Console.WriteLine("We're still connected. Retry later");
            else
            {
                if(Tell)
                    Console.WriteLine("We're disconnected");
                isConnectionProcess = false;
            }
        }

        public Boolean checkConnection()
        {
            // 연결 상태 체크 TcpClient형의 객체명 : client 
            IPGlobalProperties ipProperties = IPGlobalProperties.GetIPGlobalProperties();

            TcpConnectionInformation[] tcpConnections = new TcpConnectionInformation[1];
            Boolean return_value = false;

            try
            {
                tcpConnections = ipProperties.GetActiveTcpConnections().Where(x => x.LocalEndPoint.Equals(client.LocalEndPoint)
                                                                                                    && x.RemoteEndPoint.Equals(client.RemoteEndPoint)).ToArray();

            }
            catch
            {
                // Exception 처리 -> 여기서는 Disconnected된 것으로 보면 된다.
                return_value = false;
            }

            if (tcpConnections != null && tcpConnections.Length > 0)

            {
                TcpState stateOfConnection = tcpConnections.First().State;
                if (stateOfConnection == TcpState.Established)
                {
                    //Connected ~~
                    return_value = true;
                }
                else
                {
                    //Disconnected ~~
                    return_value = false;
                }
            }

            if (return_value == true && client.Connected == false)
                return_value = false;

            return return_value;
        }

        public Boolean Connected
        {
            get
            {
                return client.Connected;
            }
        }

        public Boolean isConnectionProcessOK
        {
            get
            {
                return isConnectionProcess;
            }
        }

        public UInt64 noPing
        {
            get
            {
                return _noPingPong_Count;
            }
        }

        public void Receive(Boolean isBlocking = true)
        {
            if (!checkConnection())
            {
                Console.WriteLine("Receive : Socket must be connected with server.");
                return;
            }

            try
            {
                // Create the state object.  
                StateObject state = new StateObject();
                state.workSocket = client;

                // Begin receiving the data from the remote device.  
                client.BeginReceive(state.buffer, 0, StateObject.BufferSize, SocketFlags.None, new AsyncCallback(ReceiveCallback), state);
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }

            if (isBlocking)
                receiveDone.WaitOne(); // blocking걸면 callback함수를 사용하지 않고 ref를 사용해서 전달해도 된다.
        }

        public void Send(byte[] byteData, int len = 0, Boolean isBlocking = true, Boolean isReceive = false)
        {
            if (!checkConnection())
            {
                Console.WriteLine("Send : Socket must be connected with server.");
                return;
            }

            if (!isReceive)
                Receive(false);

            if (len < 1)
                len = byteData.Length;

            // Begin sending the data to the remote device.  
            client.BeginSend(byteData, 0, len, 0, new AsyncCallback(SendCallback), client);

            if (isBlocking)
                sendDone.WaitOne();
        }



        private void ConnectCallback(IAsyncResult ar)
        {
            try
            {
                // Retrieve the socket from the state object.  
                client = (Socket)ar.AsyncState;

                // Complete the connection.  
                client.EndConnect(ar);

                //Console.WriteLine("Socket connected to {0}", client.RemoteEndPoint.ToString());

                // Signal that the connection has been made.  
                connectDone.Set();
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }
        private static void ReceiveCallback(IAsyncResult ar)
        {
            // Retrieve the state object and the client socket   
            // from the asynchronous state object.  
            StateObject state = (StateObject)ar.AsyncState;
            Socket _client = state.workSocket; // ?? 생략해도 될것같다는 생각이 듬.

            // Read data from the remote device.  
            int bytesRead;
            byte[] msgByte;

            try
            {
                bytesRead = _client.EndReceive(ar);
                if (bytesRead > 0)
                {
                    // There might be more data, so store the data received so far.  
                    state.sb.Append(Encoding.ASCII.GetString(state.buffer, 0, bytesRead));

                    // Get the rest of the data.  
                    _client.BeginReceive(state.buffer, 0, StateObject.BufferSize, 0,
                        new AsyncCallback(ReceiveCallback), state);
                }
                else
                {
                    // All the data has arrived; put it in response.  
                    if (state.sb.Length > 1)
                    {
                        msgByte = Encoding.ASCII.GetBytes(state.sb.ToString());
                        Receive_CallBack?.Invoke(msgByte);

                        checkPingPong(msgByte);
                        // Console.WriteLine(Encoding.Default.GetString(msgByte));
                    }
                    // Signal that all bytes have been received.  
                    receiveDone.Set();
                }
            }
            catch (Exception e)
            {
                Console.WriteLine("Receive error {0}", e.Message);
            }
        }
        private void SendCallback(IAsyncResult ar)
        {
            try
            {
                // Retrieve the socket from the state object.  
                Socket _client = (Socket)ar.AsyncState;

                // Complete sending the data to the remote device.  
                int bytesSent = _client.EndSend(ar);
                Console.WriteLine("\nSent {0} bytes to server.", bytesSent);

                // Signal that all bytes have been sent.  
                sendDone.Set();
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }
        private void DisconnectCallback(IAsyncResult ar)
        {
            // Complete the disconnect request.
            Socket client = (Socket)ar.AsyncState;
            client.EndDisconnect(ar);

            // Signal that the disconnect is complete.
            disconnectDone.Set();

        }

        private static Boolean checkPingPong(byte[] buffer)
        {
            Boolean return_value = false;
            if (buffer.Length == 8 && Encoding.Default.GetString(buffer) == "PingPong")
            {
                 Console.WriteLine("Received response");
                _noPingPong_Count = 0;
                return_value = true;
            }
            else
                _noPingPong_Count++;


            PingPong_CallBack?.Invoke(return_value);

            return return_value;
        }


        public delegate void PingPong_function(Boolean isPingPong);
        private static PingPong_function PingPong_CallBack;

        public void setCallBack_PingPong(PingPong_function func)
        {
            PingPong_CallBack = func;
        }
    }
}
