import classnames from 'classnames';
import React from 'react';
import { Line } from 'react-chartjs-2';
import { Card, CardBody, CardTitle, Col, Nav, NavItem, NavLink, Row, TabContent, TabPane } from 'reactstrap';
import getInstanceFirebase from '../../firebase/firebase';
import DUST from './../../assets/img/icon/dust.jpg';
import HUMIDITY from './../../assets/img/icon/humidity.svg';
import TEMPERATURE from './../../assets/img/icon/temperature.svg';
import moment from "moment";
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
} from 'chart.js';
interface Props { }
interface State {
  activeTab: string,
  listMacAddress: string[],
  data: Data[],
  listTime: string[],
  listHumidity: number[]
  listTemperature: number[],
  listDust: number[]
}
interface Data {
  dust: string
  humidity: string
  temperature: string
  time: string
}
const firebase = getInstanceFirebase();

ChartJS.register(
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend
);
const options = {
  responsive: true,
  plugins: {
    legend: {
      position: 'top' as const,
    },
    title: {
      display: true,
      text: 'Biểu đồ quan trắc',
    },
  },
};
export default class Example extends React.PureComponent<Props, State> {
  constructor(props: Props) {
    super(props);
    this.toggle = this.toggle.bind(this);
    this.state = {
      activeTab: '3C6105FE6919',
      listMacAddress: [],
      data: [],
      listDust: [],
      listHumidity: [],
      listTemperature: [],
      listTime: []
    };
    this.callApi()
  }
  componentDidMount() {
    this.getMacAddress()

  }

  callApi = () => {
    setTimeout(() => {
      this.getData()
    }, 30000)
  }
  getData = async () => {
    try {

      const { activeTab } = this.state
      const res: Data[] = await firebase.getData(activeTab.toString())
      res.sort((a: Data, b: Data) => moment(a.time).format('HH:mm:ss DD/MM') > moment(b.time).format('HH:mm:ss DD/MM') ? 1 : -1);
      
      this.setState({
        data: res,
        listTime: res.map((value: Data, index: number) => moment(value.time).format('HH:mm DD/MM')),
        listTemperature: res.map((value: Data) => Number(value.temperature)),
        listHumidity: res.map((value: Data) => Number(value.humidity)),
        listDust: res.map((value: Data) => Number(value.dust)),
      })
      console.log(res);


    } catch (error) {
      console.log(error);

    }
  }
  getMacAddress = async () => {
    try {
      const res: any[] = await firebase.getMacAddress()
      this.setState({ listMacAddress: res, activeTab: res[0].value }, this.getData)
    } catch (error) {
      console.log(error);
    }
  }
  toggle(tab: string) {
    if (this.state.activeTab !== tab) {
      this.setState({
        activeTab: tab
      }, this.getData);
    }
  }
  _renderNavBar = (mac: string, index: number) => {
    const { activeTab } = this.state
    return (
      <>

        <NavItem>
          <NavLink
            className={classnames({ active: activeTab === mac })}
            onClick={() => { this.toggle(mac) }}
          >
            Thiết bị {index + 1}
          </NavLink>
        </NavItem>
      </>
    )
  }
  _renderTabContent = (mac: string) => {
    const { data, listDust, listHumidity, listTemperature, listTime } = this.state
    const labels = listTime
    const length = data.length

    const datas = {
      labels,
      datasets: [
        {
          label: 'Nhiệt độ',
          data: listTemperature,
          borderColor: 'red',
          backgroundColor: 'red',
        },
        {
          label: 'Độ ẩm',
          data: listHumidity,
          borderColor: 'rgb(53, 162, 235)',
          backgroundColor: 'rgb(53, 162, 235)',
        },
        {
          label: 'Nồng độ bụi',
          data: listDust,
          borderColor: 'rgb(17, 9, 42)',
          backgroundColor: 'rgb(17, 9, 42)',
        },
      ],
    }
    return (
      <>
        <TabPane tabId={mac}>
          <Row>
            <Col lg='4' md='4' sm='12'>
              <Card className={'card-stats'} id={'temperature-popover'}>
                <CardBody>
                  <Row>
                    <Col md='4' xs='5'>
                      <div className='icon-big text-center icon-warning icon-card'>
                        <img src={TEMPERATURE} alt='' />
                      </div>
                    </Col>
                    <Col md='8' xs='7'>
                      <div className='numbers'>
                        <CardTitle tag='p'>Nhiệt độ</CardTitle>
                        <p className='card-category'>{
                        listTemperature[length - 1] ? listTemperature[length - 1].toFixed(1) : 0}°C</p>
                      </div>
                    </Col>
                  </Row>
                </CardBody>
              </Card>
            </Col>
            <Col lg='4' md='4' sm='12'>
              <Card className='card-stats'>
                <CardBody>
                  <Row>
                    <Col md='4' xs='5'>
                      <div className='icon-big text-center icon-warning icon-card'>
                        <img src={HUMIDITY} alt='' />
                      </div>
                    </Col>
                    <Col md='8' xs='7'>
                      <div className='numbers'>
                        <CardTitle tag='p'>Độ ẩm</CardTitle>
                        <p className='card-category'>{listHumidity[length - 1] ? listHumidity[length - 1] : 0} %</p>
                      </div>
                    </Col>
                  </Row>
                </CardBody>
              </Card>
            </Col>
            <Col lg='4' md='4' sm='12'>
              <Card className='card-stats'>
                <CardBody>
                  <Row>
                    <Col md='4' xs='5'>
                      <div className='icon-big text-center icon-warning icon-card'>
                        <img src={DUST} alt='' />
                      </div>
                    </Col>
                    <Col md='8' xs='7'>
                      <div className='numbers'>
                        <CardTitle tag='p'>Bụi trong không khí</CardTitle>
                        <p className='card-category'>{listDust[length - 1] ? listDust[length - 1] : 0} mg/m3</p>
                      </div>
                    </Col>
                  </Row>
                </CardBody>
              </Card>
            </Col>
            <Col xl='2' />
            <Col xl='8'>
              <Line options={options} data={datas} />
            </Col>
          </Row>
        </TabPane>
      </>
    )
  }
  render() {
    const { listMacAddress, activeTab, data } = this.state
    return (
      <div className='m-3'>
        <h2 className='d-flex justify-content-center pt-4 '>Trạm quan trắc</h2>
        <Nav tabs>
          {listMacAddress.map((mac: any, index: number) => this._renderNavBar(mac.value, index))}
        </Nav>
        {
          data.length > 0 &&
          <TabContent activeTab={activeTab}>
            {listMacAddress.map((mac: any, index: number) => this._renderTabContent(mac.value))}
          </TabContent>
        }
      </div>
    );
  }
}