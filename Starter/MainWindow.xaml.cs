using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace Starter
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {

        public MainWindow()
        {
            InitializeComponent();
        }

        private Dictionary<string, string> dict;

        private void initDataFromSettingFile()
        {

            dict = new Dictionary<string, string>();
            try
            {
                string[] items = File.ReadAllLines(@"setting.ini");
                foreach (var it in items)
                {
                    string[] keyValue = it.Split('=');
                    if (keyValue.Length < 2)
                    {
                        continue;
                    }
                    else
                    {
                        dict[keyValue[0]] = keyValue[1];
                    }
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
                //                throw;
            }
        }

        private string getSettingVal(string key)
        {
            string res;
            return dict.TryGetValue(key, out res) ? res : "";
        }

        private void Window_Initialized(object sender, EventArgs e)
        {
            initDataFromSettingFile();
            remoteServerIP.Text = getSettingVal("remoteServerIP");
            remoteServerPort.Text = getSettingVal("remoteServerPort");
            isSetMyComputerToServer.IsChecked = getSettingVal("isSetMyComputerToServer") == "true";
            openPort.Text = getSettingVal("openPort");
            robotStart.IsChecked = getSettingVal("robotStart") == "true";
            robotNumber.Text = getSettingVal("robotNumber");
            groupeSize.Text = getSettingVal("groupeSize");
        }

        private void saveSetting()
        {
            var utf8WithoutBom = new System.Text.UTF8Encoding(false);

            string[] lines = new string[7];
            lines[0] = "remoteServerIP=" + remoteServerIP.Text;
            lines[1] = "remoteServerPort=" + remoteServerPort.Text;
            lines[2] = "isSetMyComputerToServer=" +
                       ((bool)isSetMyComputerToServer.IsChecked ? "true" : "false");
            lines[3] = "openPort=" + openPort.Text;
            lines[4] = "robotStart=" + ((bool)robotStart.IsChecked ? "true" : "false");
            lines[5] = "robotNumber=" + robotNumber.Text;
            lines[6] = "groupeSize=" + groupeSize.Text;

            System.IO.File.WriteAllLines(@"setting.ini", lines, utf8WithoutBom);
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            saveSetting();
        }

        private void button_Click(object sender, RoutedEventArgs e)
        {
            saveSetting();
            try
            {
                System.Diagnostics.Process.Start("BiRoad");
            }
            catch (Exception exception)
            {
                Console.WriteLine(exception);
            }
        }
    }
}
